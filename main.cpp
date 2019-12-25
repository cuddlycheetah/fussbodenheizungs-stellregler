#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

#define TB6600_0_ENABLE 10
#define TB6600_0_DIR 11
#define TB6600_0_STEP 12

#define TB6600_1_ENABLE 10
#define TB6600_1_DIR 11
#define TB6600_1_STEP 12


//#define D_SPEED_FAST 4000
#define D_SPEED_FAST 7400

void setup() {
  while (!Serial);
  Serial.begin(230400);
  pinMode(13, OUTPUT);
  pinMode(TB6600_0_ENABLE, OUTPUT);
  pinMode(TB6600_0_DIR, OUTPUT);
  pinMode(TB6600_0_STEP, OUTPUT);
  pinMode(TB6600_1_ENABLE, OUTPUT);
  pinMode(TB6600_1_DIR, OUTPUT);
  pinMode(TB6600_1_STEP, OUTPUT);  
  initADC();
}

#define degreesToRadians(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define radiansToDegrees(angleRadians) ((angleRadians) * 180.0 / M_PI)

#define STEP_FREQ_SLOW 3500
#define STEP_FREQ_FAST 2000

#define POT_0_MIN 650
#define POT_0_MAX 1650
int32_t ADC0[5] =  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
int32_t ADC0S;
int potProzent0 = 0;

#define POT_1_MIN 0
#define POT_1_MAX 2400
int32_t ADC1[5] =  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
int32_t ADC1S;
int potProzent1 = 0;

void initADC() {
  ads.begin();
  //                                                          ADS1015  ADS1115
  //                                                          -------  -------
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  //ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  //ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  //ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  initADC0();
  initADC1();
}
void initADC0() {
  int16_t val = ads.readADC_SingleEnded(0);
  ADC0[4] = val;
  ADC0[3] = val;
  ADC0[2] = val;
  ADC0[1] = val;
  ADC0[0] = val;
  ADC0S = val;
  potProzent0 = map(ADC0S / 10, POT_0_MIN, POT_0_MAX, 0, 1024);
}
void initADC1() {
  int16_t val = ads.readADC_SingleEnded(1);
  ADC1[4] = val;
  ADC1[3] = val;
  ADC1[2] = val;
  ADC1[1] = val;
  ADC1[0] = val;
  ADC1S = val;
  potProzent1 = map(ADC1S / 10, POT_1_MIN, POT_1_MAX, 0, 1024);
}
void readADC0() {
  ADC0[4] = ADC0[3];
  ADC0[3] = ADC0[2];
  ADC0[2] = ADC0[1];
  ADC0[1] = ADC0[0];
  ADC0[0] = ads.readADC_SingleEnded(0);
  ADC0S = (ADC0[0] + ADC0[1] + ADC0[2] + ADC0[3] + ADC0[4]) / 5;
  potProzent0 = map(ADC0S / 10, POT_0_MIN, POT_0_MAX, 0, 1024);
}
void readADC1() {
  ADC1[4] = ADC1[3];
  ADC1[3] = ADC1[2];
  ADC1[2] = ADC1[1];
  ADC1[1] = ADC1[0];
  ADC1[0] = ads.readADC_SingleEnded(1);
  ADC1S = (ADC1[0] + ADC1[1] + ADC1[2] + ADC1[3] + ADC1[4]) / 5;
  potProzent1 = map(ADC1S / 10, POT_1_MIN, POT_1_MAX, 0, 1024);
}

void goto0(int target) {
  digitalWrite(TB6600_0_ENABLE, HIGH);
  double t = 800;
  delay(300);
  int i = 0;
  int turnOvers = 0;
  bool lastDirection = 0;
  long steps = 0;

  while ( abs(potProzent0 - target) >= 1) {
    boolean nowDirection = (potProzent0 - target) > 0;

    if (nowDirection != lastDirection) turnOvers++;

    digitalWrite(TB6600_0_DIR, nowDirection);
    digitalWrite(TB6600_0_STEP, HIGH);
    t = map( abs(potProzent0 - target), 0, 200, 1, 300);
    if (t > 50) t = D_SPEED_FAST;
    else t = 20;
    t = floor(t/2) * 2;
    steps++;

    delayMicroseconds(1000000 / t);
    digitalWrite(TB6600_0_STEP, LOW);
    delayMicroseconds(1000000 / t);

    if ((t > 50 && i % 50 == 0) || (t <= 50 && i % 10 == 0)) {
      readADC0();
    }
    i++;

    lastDirection = nowDirection;

    if (turnOvers > 10) {
      Serial.println("F#MAXCORRECTIONS");
      break;
    }
  }

  delay(250);
  digitalWrite(TB6600_0_ENABLE, LOW);
  Serial.println(steps);
}

void goto1(int target) {
  digitalWrite(TB6600_1_ENABLE, HIGH);
  double t = 800;
  delay(300);
  int i = 0;
  int turnOvers = 0;
  bool lastDirection = 0;
  long steps = 0;

  while ( abs(potProzent1 - target) >= 1) {
    boolean nowDirection = (potProzent1 - target) > 0;

    if (nowDirection != lastDirection) turnOvers++;

    digitalWrite(TB6600_1_DIR, nowDirection);
    digitalWrite(TB6600_1_STEP, HIGH);
    t = map( abs(potProzent0 - target), 0, 200, 1, 300);
    if (t > 50) t = D_SPEED_FAST;
    else t = 20;
    t = floor(t/2) * 2;
    steps++;

    delayMicroseconds(1000000 / t);
    digitalWrite(TB6600_1_STEP, LOW);
    delayMicroseconds(1000000 / t);

    if ((t > 50 && i % 50 == 0) || (t <= 50 && i % 10 == 0)) {
      readADC1();
    }
    i++;

    lastDirection = nowDirection;

    if (turnOvers > 10) {
      Serial.println("F#MAXCORRECTIONS");
      break;
    }
  }
  delay(2500);
  digitalWrite(TB6600_1_ENABLE, LOW);
  Serial.println(steps);
}

void loop() {
  int16_t adc1, adc2, adc3;
  readADC0();
  //Serial.println(ADC0S / 10);
  Serial.print('A');
  Serial.println(potProzent0);
  Serial.print('B');
  Serial.println(potProzent1);
  //adc1 = ads.readADC_SingleEnded(1);
  //adc2 = ads.readADC_SingleEnded(2);
  //adc3 = ads.readADC_SingleEnded(3);
  //Serial.print("AIN0: "); 
  //Serial.print("AIN1: "); Serial.println(adc1);
  //Serial.print("AIN2: "); Serial.println(adc2);
  //Serial.print("AIN3: "); Serial.println(adc3);
  //Serial.println(" ");
  if (Serial.available() > 0) {
    String line = Serial.readString();
    if (line.indexOf('#') > -1) {
      String command = line.substring(0, line.indexOf('#'));
      String args = line.substring(line.indexOf('#') + 1);
      if (command.equalsIgnoreCase(F("A")) || command.equalsIgnoreCase(F("B"))) {
        String arg0 = args.substring(0, args.indexOf('#'));
        String arg1 = args.substring(args.indexOf('#') + 1);
        if (atoi(arg0.c_str()) != 1000 - atoi(arg1.c_str()))  {
          Serial.println(F("F#CHECKSUM"));
          return;
        }
        int rotation = atoi( arg0.c_str() );
        Serial.println(rotation);
        rotation = min(max(rotation, 0), 1000);
        if (command.equalsIgnoreCase(F("A"))) goto0(rotation);
        if (command.equalsIgnoreCase(F("B"))) goto1(rotation);
      }
    }
  }
  delay(100 * 1);
}
