#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
#define TB6600_MASTER_DIR 8
#define TB6600_MASTER_STEP 9

#define POT_0_MIN 650
#define POT_0_MAX 1650
#define TB6600_0_ENABLE 12

#define POT_1_MIN 0
#define POT_1_MAX 2400
#define TB6600_1_ENABLE 10

#define D_SPEED_FAST 8000

void setup() {
  while (!Serial);
  Serial.begin(230400);
  pinMode(13, OUTPUT);

  pinMode(TB6600_MASTER_DIR, OUTPUT);
  pinMode(TB6600_MASTER_STEP, OUTPUT);
  pinMode(TB6600_0_ENABLE, OUTPUT);
  pinMode(TB6600_1_ENABLE, OUTPUT);
  delay(100);

  tone(TB6600_0_ENABLE, 500);
  delay(200);
  noTone(TB6600_0_ENABLE); 

  digitalWrite(TB6600_0_ENABLE, LOW);
  digitalWrite(TB6600_1_ENABLE, LOW);

  initADC();
}

int32_t ADC0[5] =  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
int32_t ADC0S;
int32_t ADC1[5] =  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
int32_t ADC1S;

int potProzent = 0;

void initADC() {
  ads.begin();
  //                                                             ADS1015  ADS1115
  //                                                             -------  -------
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain   +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  //ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  //ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  //ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  initADC0();
  initADC1();
  tone(TB6600_0_ENABLE, 600);
  delay(200);
  tone(TB6600_0_ENABLE, 400);
  delay(200);
  noTone(TB6600_0_ENABLE); 

  digitalWrite(TB6600_0_ENABLE, LOW);
  digitalWrite(TB6600_1_ENABLE, LOW);


}
void startProtectedMode() {
  while (true) {
    Serial.println(F("F#FATAL_ADC_FAIL"));
    yield;
    delay(1000);
    tone(TB6600_0_ENABLE, 500);
    delay(500);
    noTone(TB6600_0_ENABLE);
    digitalWrite(TB6600_0_ENABLE, LOW);
  }
}
void initADC0() {
  int16_t val = ads.readADC_SingleEnded(0);
  ADC0[4] = val;
  ADC0[3] = val;
  ADC0[2] = val;
  ADC0[1] = val;
  ADC0[0] = val;
  ADC0S = val;
  if (val == -1) { Serial.println("ADC not working"); startProtectedMode(); }
  potProzent = map(ADC0S / 10, POT_0_MIN, POT_0_MAX, 0, 1024);
}
void initADC1() {
  int16_t val = ads.readADC_SingleEnded(1);
  ADC1[4] = val;
  ADC1[3] = val;
  ADC1[2] = val;
  ADC1[1] = val;
  ADC1[0] = val;
  ADC1S = val;
  if (val == -1) { Serial.println("ADC not working"); startProtectedMode(); } 
  potProzent = map(ADC1S / 10, POT_1_MIN, POT_1_MAX, 0, 1024);
}
void readADC(int driverId) {
  if (driverId == 0) readADC0();
  if (driverId == 1) readADC1();
}

void readADC0() {
  ADC0[4] = ADC0[3];
  ADC0[3] = ADC0[2];
  ADC0[2] = ADC0[1];
  ADC0[1] = ADC0[0];
  ADC0[0] = ads.readADC_SingleEnded(0);
  ADC0S = (ADC0[0] + ADC0[1] + ADC0[2] + ADC0[3] + ADC0[4]) / 5;
  potProzent = map(ADC0S / 10, POT_0_MIN, POT_0_MAX, 0, 1024);
  if (potProzent > 2048) { Serial.println("ADC not working"); startProtectedMode(); } 
}
void readADC1() {
  ADC1[4] = ADC1[3];
  ADC1[3] = ADC1[2];
  ADC1[2] = ADC1[1];
  ADC1[1] = ADC1[0];
  ADC1[0] = ads.readADC_SingleEnded(1);
  ADC1S = (ADC1[0] + ADC1[1] + ADC1[2] + ADC1[3] + ADC1[4]) / 5;
  potProzent = map(ADC1S / 10, POT_1_MIN, POT_1_MAX, 0, 1024);
  if (potProzent > 2048) { Serial.println("ADC not working"); startProtectedMode(); } 
}

void gotoTarget(int target, int driverId) {
  digitalWrite(TB6600_0_ENABLE, driverId == 0);
  digitalWrite(TB6600_1_ENABLE, driverId == 1);
  double t = 800;
  int i = 0;
  int turnOvers = 0;
  bool lastDirection = 0;
  long steps = 0;

  readADC(driverId);
  int startProzent = potProzent + 0;
  long startMillis = millis();

  while ( abs(potProzent - target) >= 1) {
    boolean nowDirection = (potProzent - target) > 0;
    if (millis() - startMillis > 30 * 1000) break; 

    if (nowDirection != lastDirection) turnOvers++;

    t = map( abs(potProzent - target), 0, 200, 1, 300);
    t = t > 50 ? D_SPEED_FAST : 20;
    t = floor(t/2) * 2;
    steps++;

    // rotate 1 step
    digitalWrite(TB6600_MASTER_DIR, nowDirection);
    digitalWrite(TB6600_MASTER_STEP, HIGH);
    delayMicroseconds(1000000 / t);
    digitalWrite(TB6600_MASTER_STEP, LOW);
    delayMicroseconds(1000000 / t);
    // read results sometimes
    if ((t > 50 && i % 50 == 0) || (t <= 50 && i % 10 == 0)) {
      readADC(driverId);
    }
    i++;

    lastDirection = nowDirection;

    if (turnOvers > 10) {
      Serial.println("F#MAXCORRECTIONS");
      break;
    }
  }
  digitalWrite(TB6600_0_ENABLE, LOW);
  digitalWrite(TB6600_1_ENABLE, LOW);
}

void loop() {
  readADC0();
  Serial.print('A');
  Serial.println(potProzent);
  readADC1();
  Serial.print('B');
  Serial.println(potProzent);

  if (Serial.available() > 0) {
    String line = Serial.readString();
    if (line.indexOf('#') > -1) {
      String command = line.substring(0, line.indexOf('#'));
      String args = line.substring(line.indexOf('#') + 1);
      // set cmd
      if (command.equalsIgnoreCase(F("A")) || command.equalsIgnoreCase(F("B"))) {
        String arg0 = args.substring(0, args.indexOf('#'));
        String arg1 = args.substring(args.indexOf('#') + 1);
        if (atoi(arg0.c_str()) != 1000 - atoi(arg1.c_str()))  {
          Serial.println(F("F#CHECKSUM"));
          return;
        }
        if (ads.readADC_SingleEnded(0) == -1) { Serial.println("ADC not working"); startProtectedMode(); }
 
        int rotation = atoi( arg0.c_str() );
        rotation = min(max(rotation, 0), 1000);
        if (command.equalsIgnoreCase(F("A"))) gotoTarget(rotation, 0);
        if (command.equalsIgnoreCase(F("B"))) gotoTarget(rotation, 1);
      }
      // debug cmd
      if (command.equalsIgnoreCase(F("D"))) {
        Serial.print("D#millis"); Serial.println(millis());
        Serial.print("D#adc0"); Serial.println(ADC0S);
        Serial.print("D#adc1"); Serial.println(ADC1S);
      }
    }
  }
  delay(50 * 1);
}
