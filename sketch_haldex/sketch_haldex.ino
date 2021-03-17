#include <math.h>
#include <Stepper.h>

Stepper motor(200, 2, 3, 4, 5);

#define MOTOR_STEPS 80

#define NTC_B 3330.0
#define NTC_T0 300.0
#define NTC_R0 4700.0
#define R_DIVIDER 2200.0

#define ADC_TO_R(adc) ((R_DIVIDER * adc) / (1023 - adc))
#define R_TO_TEMP(r) ((NTC_T0 * NTC_B / (NTC_T0 * log((float)r/NTC_R0) + NTC_B)) - 275.0)

void setup() {
  // set the speed at 60 rpm:
  motor.setSpeed(120);
  // initialize the serial port:
  Serial.begin(9600);

  // Ground pin for NTC
  pinMode(A1, OUTPUT);
  digitalWrite(A1, 0);
}

void loop() {
  // step one revolution  in one direction:
  //valve_on();
  //delay(500);

  // step one revolution in the other direction:
  //valve_off();
  //delay(2000);

  int adc = analogRead(A0);
  float r = ADC_TO_R(adc);
  float temp = R_TO_TEMP(r);
  Serial.println(temp);
  delay(1000);
}

void valve_on()
{
  motor.step(MOTOR_STEPS);
}

void valve_off()
{
  motor.step(-MOTOR_STEPS);

  // release the motor
  digitalWrite(2, 0);
  digitalWrite(3, 0);
  digitalWrite(4, 0);
  digitalWrite(5, 0);
}
