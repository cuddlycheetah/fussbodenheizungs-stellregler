#include <LM75A.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>


LM75A lm75a_sensor(true,  //A0 LM75A pin state
                   true,  //A1 LM75A pin state
                   true); //A2 LM75A pin state


Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

#define TB6600_ENABLE 10
#define TB6600_DIR 11
#define TB6600_STEP 12

void setup() {
  while (!Serial); // Leonardo: wait for serial monitor  
  Serial.begin(230400);
  //Serial.println(F("M2M_LM75A - Basic usage"));
  //Serial.println(F("==========================================="));
  //Serial.println("");
  pinMode(TB6600_ENABLE, OUTPUT);
  pinMode(TB6600_DIR, OUTPUT);
  pinMode(TB6600_STEP, OUTPUT);
  //Serial.print(F("Temperature in Celsius: "));
  //Serial.print(lm75a_sensor.getTemperatureInDegrees());
  //Serial.println(F(" *C"));
  
  ads.begin();
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
   ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
   //ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  //ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  
}
#define degreesToRadians(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define radiansToDegrees(angleRadians) ((angleRadians) * 180.0 / M_PI)

#define STEP_FREQ_SLOW 3500
#define STEP_FREQ_FAST 2000

#define POT_MIN 650
#define POT_MAX 1650

int32_t ADC0[5] =  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
int32_t ADC0S;
int potProzent = 0;
void readADC() {
  ADC0[4] = ADC0[3];
  ADC0[3] = ADC0[2];
  ADC0[2] = ADC0[1];
  ADC0[1] = ADC0[0];
  ADC0[0] = ads.readADC_SingleEnded(0);
  ADC0S = (ADC0[0] + ADC0[1] + ADC0[2] + ADC0[3] + ADC0[4]) / 5;
  potProzent = map(ADC0S / 10, POT_MIN, POT_MAX, 0, 1024);
  //Serial.println(ADC0[0] );
}
void drive0(int degree) {
  digitalWrite(TB6600_DIR, degree > 0);
  digitalWrite(TB6600_ENABLE, HIGH);
  double t = 800;
  delay(300);
  long DO_STEPS = (abs(degree) / 360.0) * 5710.0;
  long NORM_SPEED = map(1, 0, 2, STEP_FREQ_FAST, STEP_FREQ_SLOW);
  long START_BEREICH = DO_STEPS / 20;
  long END_BEREICH = DO_STEPS / 10;
  for (long i = 0; i < DO_STEPS; i++) {
    long iM = i;
    if (i > DO_STEPS - END_BEREICH) {
      iM = 0;
      t = map(i - (DO_STEPS - END_BEREICH), 0, END_BEREICH, NORM_SPEED, STEP_FREQ_SLOW / 10);
    } else if (i < START_BEREICH) {
      t = map(i, 0, START_BEREICH, STEP_FREQ_SLOW / 10, NORM_SPEED);
    } else {
      t = map(DO_STEPS / 2, 0, DO_STEPS, STEP_FREQ_FAST, STEP_FREQ_SLOW);
    }
    if (abs(degree) < 45) t = 1;
    digitalWrite(TB6600_STEP, HIGH);
    delayMicroseconds(1000000 / t);
    digitalWrite(TB6600_STEP, LOW);
    delayMicroseconds(1000000 / t);
    if (i % 10 == 0) {
      readADC();
      //Serial.println(ADC0S);
      //Serial.print(i);
      //Serial.print(F("/"));
      //Serial.println(DO_STEPS);
    }
  }
  delay(500);
  digitalWrite(TB6600_ENABLE, LOW);
}
void goto0(int target) {
  digitalWrite(TB6600_ENABLE, HIGH);
  double t = 800;
  delay(300);
  int i = 0;
  long steps = 0;
  while ( abs(potProzent - target) >= 1) {
    digitalWrite(TB6600_DIR, (potProzent - target) > 0);
    digitalWrite(TB6600_STEP, HIGH);
    t = map( abs(potProzent - target) ,0, 200, 5, 600);
    if (t > 50) t = 900;
    t = floor(t/2) * 2;
    steps++;

    delayMicroseconds(1000000 / t);
    digitalWrite(TB6600_STEP, LOW);
    delayMicroseconds(1000000 / t);
    if ((t > 50 && i % 50 == 0) || (t <= 50 && i % 10 == 0)) {
      readADC();
      Serial.println(potProzent);
      Serial.print(F("/"));
      Serial.print(target);
      Serial.print(F("/"));
      Serial.println(t);
    }
    i++;
  }
  delay(2500);
  digitalWrite(TB6600_ENABLE, LOW);
  Serial.println(steps);
}
void loop() {
  //Serial.print(F("Temperature in Celsius: "));
  //Serial.print(lm75a_sensor.getTemperatureInDegrees());
  //Serial.println(F(" *C"));
  
  int16_t adc1, adc2, adc3;
  readADC() ;
  //Serial.println(ADC0S / 10);
  Serial.println(potProzent);
  //adc1 = ads.readADC_SingleEnded(1);
  //adc2 = ads.readADC_SingleEnded(2);
  //adc3 = ads.readADC_SingleEnded(3);
  //Serial.print("AIN0: "); 
  //Serial.print("AIN1: "); Serial.println(adc1);
  //Serial.print("AIN2: "); Serial.println(adc2);
  //Serial.print("AIN3: "); Serial.println(adc3);
  //Serial.println(" ");
  if (Serial.available() > 0) {
    String s = Serial.readString();
    int rotation = atoi(s.c_str());
    rotation = min(max(rotation,0),1000);
    Serial.println(s);
    goto0(rotation);
  }
  delay(100 * 1);
}
