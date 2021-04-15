#include <math.h>
#include <A4988.h>
#include <arduino-timer.h>

// ======== CONFIG ========
#define A4988_EN 2
#define A4988_STEP 3
#define A4988_DIR 4
#define NTC_PIN A0              // temperature sensor pin
#define NTC_GND A1              // temperature sensor ground pin
#define RELAY_PIN 7             // pin for pump relay
#define BTN_PIN 8               // pin for in-car button
#define LED_PIN 9               // pin for status LED (near the button)

#define MOTOR_STEPS 80          // ammount of motor rotation (generally don't need to edit this)
#define ALARM_TEMP 95           // limit temperature at which the valve will be shut off automatically to prevent overheating
//#define REVERSE_MOTOR         // uncomment to reverse motor direction
//#define IGNORE_TEMP           // uncomment to disable temperature monitoring
#define R_DIVIDER 2200.0        // upper resistor connected from NTC_PIN to VCC
#define SHORT_ON_TIME 90 * 1000 //   time for turning on by short button press (in milliseconds)
// ========================

// NTC parameters (default values for stock Haldex thermistor)
#define NTC_B 3330.0
#define NTC_T0 300.0
#define NTC_R0 4700.0

#define ADC_TO_R(adc) ((R_DIVIDER * adc) / (1023 - adc))
#define R_TO_TEMP(r) ((NTC_T0 * NTC_B / (NTC_T0 * log((float)r/NTC_R0) + NTC_B)) - 275.0)

A4988 stepper(200, A4988_DIR, A4988_STEP);
Timer<> timer = timer_create_default();
Timer<>::Task btn_hold_task = NULL;
Timer<>::Task awd_off_task = NULL;
Timer<>::Task alarm_task = NULL;
bool awd_is_on = false;
bool alarm = false;
bool prev_btn = false;
bool new_key_press = false;

void setup()
{
  Serial.begin(9600);

  // 120 RPM / full-step mode
  stepper.begin(120, 1);

  // disable stepper
  pinMode(A4988_EN, OUTPUT);
  digitalWrite(A4988_EN, 1);

  timer.in(400, alarm_check);
}

void loop()
{
  delay(50);

  timer.tick();

  if (btn_is_pressed()) {
    if (!prev_btn)
      on_btn_press();
    prev_btn = true;
  } else {
    if (prev_btn)
      on_btn_release();
    prev_btn = false;
  }
}

void on_btn_press()
{
  if (awd_is_on == false) {
    if (!alarm)
      btn_hold_task = timer.in(1000, awd_on);
  }
  new_key_press = true;
}

void on_btn_release()
{
  if (awd_is_on == false) {
    timer.cancel(btn_hold_task);
    if (!alarm) {
      awd_on();
      awd_off_task = timer.in(SHORT_ON_TIME, awd_off);
    }
  } else if (awd_is_on == true) {
    if (new_key_press) {
      timer.cancel(awd_off_task);
      awd_off();
    }
  }
}

bool btn_is_pressed()
{
  return (digitalRead(BTN_PIN) == 0);
}

void awd_on()
{
  new_key_press = false;
  
  if (awd_is_on == true)
    return;
  
  pump_on();
  led_on();
  valve_on();
  
  awd_is_on = true;
}

void awd_off()
{
  if (awd_is_on == false)
    return;
    
  pump_off();
  led_off();
  valve_off();

  awd_is_on = false;
}

void alarm_check()
{
  alarm = !check_temp();
  
  if (alarm) {
    if (digitalRead(LED_PIN) == 0)
      led_on();
    else
      led_off();

    if (awd_is_on)
      awd_off();
  } else {
    if (digitalRead(LED_PIN) != 0)
      if (!awd_is_on)
        led_off();
  }

  timer.in(400, alarm_check);
}

bool check_temp()
{
#ifndef IGNORE_TEMP
  static int silent_counter = 0;
  silent_counter = (silent_counter + 1) % 4;
  
  // Enable ground pin for NTC
  pinMode(NTC_GND, OUTPUT);
  digitalWrite(NTC_GND, 0);
  delay(1);
  
  // Read temperature
  int adc = analogRead(NTC_PIN);
  if (adc > 1020 || adc < 20) {
    if (silent_counter == 0)
      Serial.println("Temp: sensor not connected");
    return true;
  }
  float r = ADC_TO_R(adc);
  float temp = R_TO_TEMP(r);
  char buf[24];
  bool ok = (temp < ALARM_TEMP);
  if (ok)
    sprintf(buf, "Temp: %d C", (int)temp);
  else
    sprintf(buf, "Temp: %d C (overheat)", (int)temp);
  if (silent_counter == 0)
    Serial.println(buf);
  
  // Disable ground pin
  pinMode(NTC_GND, INPUT);
  
  return ok;
#else
  return true
#endif
}

void led_on()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 1);
}

void led_off()
{
  digitalWrite(LED_PIN, 0);
  pinMode(LED_PIN, INPUT);
}

void pump_on()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 1);

  Serial.println("pump ON");
}

void pump_off()
{
  digitalWrite(RELAY_PIN, 0);
  pinMode(RELAY_PIN, INPUT);

  Serial.println("pump OFF");
}

void valve_on()
{
  // enable stepper
  digitalWrite(A4988_EN, 0);

#ifdef REVERSE_MOTOR
  stepper.move(-MOTOR_STEPS);
#else
  stepper.move(MOTOR_STEPS);
#endif
  
  Serial.println("valve ON");
}

void valve_off()
{
#ifdef REVERSE_MOTOR
  stepper.move(MOTOR_STEPS);
#else
  stepper.move(-MOTOR_STEPS);
#endif

  // disable stepper
  digitalWrite(A4988_EN, 1);

  Serial.println("valve OFF");
}
