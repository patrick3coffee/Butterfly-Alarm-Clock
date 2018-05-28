#include <EEPROM.h>
#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include <FastLED.h>

#define REDPIN 6
#define BLUEPIN 3
#define GREENPIN 9
#define SATURATION 140
#define TWINKLE 6
//#define DEBUG

bool colorOn;

void setup() {
  Serial.begin(9600);
  rtc.begin(10);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(A3, INPUT);

  Serial.println("Enter \"?\" to print help message");
  setStartupState();
}

void loop() {
  if (colorOn) {
    setWingColor();
  }
  else {
    showAnalogRGB( CHSV( 0, 0, 0) );
  }

  if (Serial.available()) {
    menuSelect();
  }

  if (rtc.alarm1()) {
    wakeAlarm();
  }
  else if (rtc.alarm2()) {
    sleepAlarm();
  }
  else {
    delay(50);
  }

}

void menuSelect() {
  byte menuSelection = Serial.read();
  switch (menuSelection) {
    case '?':
      printHelpMessage();
      break;
    case '1':
      userSetTime();
      break;
    case '2':
      userSetWakeAlarm();
      break;
    case '3':
      userSetSleepAlarm();
      break;
    case '4':
      sleepAlarm();
      break;
    case '5':
      wakeAlarm();
      break;
    default:
      Serial.print(menuSelection);
      Serial.println(" is an unknown command");
      break;
  }
}

int getBrightness() {
  //read brightness from room
  int reading = analogRead(A3);

  // fix brightness curve
  if (reading < 200) {
    reading = reading / 4;
  }
  else if (reading < 400) {
    reading = reading / 3;
  }
  else if (reading < 600) {
    reading = reading / 2;
  }

  uint8_t brightness = map(reading, 0, 1024, 5, 255);

#ifdef DEBUG
  Serial.print("Reading: ");
  Serial.println(reading);
  Serial.print("Brightness: ");
  Serial.println(brightness);
  delay(200);
#endif

  return brightness;
}

void setWingColor() {

  uint8_t hue, saturation, minute;

  rtc.update();
  minute = rtc.minute();

  hue = map(minute, 0, 59, 1, 255);
  saturation = SATURATION + randomTwinkle();
  // Use FastLED automatic HSV->RGB conversion
  showAnalogRGB( CHSV( hue, saturation, getBrightness()) );
}

void showAnalogRGB( const CRGB& rgb)
{
  analogWrite(REDPIN,   rgb.r );
  analogWrite(GREENPIN, rgb.g );
  analogWrite(BLUEPIN,  rgb.b );
}

int randomTwinkle() {
  int offset = random(-TWINKLE, TWINKLE);
  return offset;
}

void printHelpMessage() {
  printTime();
  Serial.println("");
  Serial.println("Enter \"1\" to set time.");
  Serial.println("Enter \"2\" to set wake alarm");
  Serial.println("Enter \"3\" to set sleep alarm");
  Serial.println("Enter \"4\" to set mode to sleep");
  Serial.println("Enter \"5\" to set mode to awake");
  Serial.println("");
  Serial.println("!!! turn off all line endings !!!");
}

void printTime() {
  //get current time
  rtc.update();
  int s = rtc.second();
  int m = rtc.minute();
  int h = rtc.hour();

  //print time
  Serial.println("");
  Serial.print("Current time is: ");
  Serial.print(h);
  Serial.print(":");
  Serial.print(m);
  Serial.print(":");
  Serial.println(s);
}

void userSetTime() {
  Serial.println("Enter current hour (24 hour format):");
  while (!Serial.available()) {
    delay(10);
  }
  int newHour = Serial.parseInt();
  Serial.println("Enter current minute:");
  while (!Serial.available()) {
    delay(10);
  }
  int newMinute = Serial.parseInt();
  rtc.setTime(0, newMinute, newHour, 0, 0, 0, 0);
  Serial.println("New time set");
}

void userSetWakeAlarm() {
  Serial.print("Current alarm time: ");
  Serial.print(EEPROM.read(1));
  Serial.print(":");
  Serial.println(EEPROM.read(0));
  Serial.println("Enter wake hour (24 hour format):");
  while (!Serial.available()) {
    delay(10);
  }
  int newHour = Serial.parseInt();
  Serial.println("Enter wake minute:");
  while (!Serial.available()) {
    delay(10);
  }
  int newMinute = Serial.parseInt();
  EEPROM.update(0, newMinute);
  EEPROM.update(1, newHour);
  rtc.setAlarm1(0, newMinute, newHour);
  Serial.println("Wake alarm set");
}

void userSetSleepAlarm() {
  Serial.print("Current alarm time: ");
  Serial.print(EEPROM.read(3));
  Serial.print(":");
  Serial.println(EEPROM.read(2));
  Serial.println("Enter sleep hour (24 hour format):");
  while (!Serial.available()) {
    delay(10);
  }
  int newHour = Serial.parseInt();
  Serial.println("Enter sleep minute:");
  while (!Serial.available()) {
    delay(10);
  }
  int newMinute = Serial.parseInt();
  EEPROM.update(2, newMinute);
  EEPROM.update(3, newHour);
  rtc.setAlarm2(newMinute, newHour);
  Serial.println("Sleep alarm set");
}

void wakeAlarm() {
  colorOn = true;
  int wakeMinute = EEPROM.read(0);
  int wakeHour = EEPROM.read(1);
  rtc.update();
  rtc.setAlarm1(0, wakeMinute, wakeHour);
  Serial.println("Wake");
}

void sleepAlarm() {
  colorOn = false;
  int sleepMinute = EEPROM.read(2);
  int sleepHour = EEPROM.read(3);
  rtc.update();
  rtc.setAlarm2(sleepMinute, sleepHour);
  Serial.println("Sleep");
}

void setStartupState() {
  rtc.update();
  int wakeMinute = EEPROM.read(0);
  int wakeHour = EEPROM.read(1);
  int sleepMinute = EEPROM.read(2);
  int sleepHour = EEPROM.read(3);
  int currentMinute = rtc.minute();
  int currentHour = rtc.hour();

  Serial.print("Startup state: ");

  if (wakeHour < currentHour && sleepHour > currentHour) {
    wakeAlarm();
  }
  else if (wakeHour == currentHour && wakeMinute <= currentMinute) {
    wakeAlarm();
  }
  else {
    sleepAlarm();
  }
}

