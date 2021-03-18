#include <math.h>
#include <Stepper.h>
#include <arduino-timer.h>

// ======== CONFIG ========
#define MOTOR_PINS 2, 3, 4, 5   // pins for stepper motor
#define NTC_PIN A0              // temperature sensor pin
#define NTC_GND A1              // temperature sensor ground pin
#define RELAY_PIN 7             // pin for pump relay
#define BTN_PIN 8               // pin for in-car button
#define LED_PIN 9               // pin for status LED (near the button)

#define MOTOR_STEPS 80          // ammount of motor rotation (generally don't need to edit this)
#define ALARM_TEMP 25           // limit temperature at which the valve will be shut off automatically to prevent overheating
//#define REVERSE_MOTOR         // uncomment to reverse motor direction
//#define IGNORE_TEMP           // uncomment to disable temperature monitoring
#define R_DIVIDER 2200.0        // upper resistor connected from NTC_PIN to VCC
#define ON_TIME 10 * 1000      // time for turning on by short button press (in milliseconds)
// ========================

// NTC parameters (default values for stock Haldex thermistor)
#define NTC_B 3330.0
#define NTC_T0 300.0
#define NTC_R0 4700.0

#define ADC_TO_R(adc) ((R_DIVIDER * adc) / (1023 - adc))
#define R_TO_TEMP(r) ((NTC_T0 * NTC_B / (NTC_T0 * log((float)r/NTC_R0) + NTC_B)) - 275.0)

Stepper motor(200, MOTOR_PINS);
Timer<> timer = timer_create_default();
Timer<>::Task btn_hold_task = NULL;
Timer<>::Task awd_off_task = NULL;
Timer<>::Task alarm_task = NULL;
bool awd_is_on = false;
bool alarm = false;
bool prev_btn = false;
bool new_key_press = false;

// https://lastminuteengineers.com/a4988-stepper-motor-driver-arduino-tutorial/

void setup()
{
  Serial.begin(9600);
  motor.setSpeed(120);

  timer.every(400, alarm_check);
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
      awd_off_task = timer.in(ON_TIME, awd_off);
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

bool alarm_check()
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

  return true;  // for timer API
}

bool check_temp()
{
#ifndef IGNORE_TEMP
  // Enable ground pin for NTC
  pinMode(NTC_GND, OUTPUT);
  digitalWrite(NTC_GND, 0);
  delay(1);
  
  // Read temperature
  int adc = analogRead(NTC_PIN);
  if (adc > 1020 || adc < 20) {
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
#ifndef REVERSE_MOTOR
  motor.step(MOTOR_STEPS);
#else
  motor.step(-MOTOR_STEPS);
#endif

  // release the motor (set all MOTOR_PINS to 0)
  int pins[] = {MOTOR_PINS};
  for (int i = 0; i < 4; i++)
    digitalWrite(pins[i], 0);

  Serial.println("valve ON");
}

void valve_off()
{
#ifndef REVERSE_MOTOR
  motor.step(-MOTOR_STEPS);
#else
  motor.step(MOTOR_STEPS);
#endif

  // release the motor (set all MOTOR_PINS to 0)
  int pins[] = {MOTOR_PINS};
  for (int i = 0; i < 4; i++)
    digitalWrite(pins[i], 0);

  Serial.println("valve OFF");
}
