#include <EEPROM.h>
#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include <FastLED.h>

#define REDPIN 6
#define BLUEPIN 3
#define GREENPIN 9
#define SATURATION 120
#define TWINKLERANGE 120
//#define DEBUG

bool colorOn;
int twinkleOffset = TWINKLERANGE / 2;

void setup() {
  Serial.begin(9600);
  rtc.begin(10);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(A3, INPUT);

  printHelpMessage();
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
#ifdef DEBUG
  delay(200);
#endif

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
      printHelpMessage();
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
#endif

  return brightness;
}

void setWingColor() {

  uint8_t hue, saturation, brightness, minute;

  randomTwinkle();

  rtc.update();
  minute = rtc.minute();

  hue = map(minute, 0, 59, 1, 255);
  saturation = SATURATION + twinkleOffset;
  brightness = getBrightness();

#ifdef DEBUG
  Serial.print("H: ");
  Serial.print(hue);
  Serial.print("   S: ");
  Serial.print(saturation);
  Serial.print("   V: ");
  Serial.println(brightness);
#endif


  // Use FastLED automatic HSV->RGB conversion
  showAnalogRGB( CHSV( hue, saturation, brightness) );
}

void showAnalogRGB( const CRGB& rgb)
{
  analogWrite(REDPIN,   rgb.r );
  analogWrite(GREENPIN, rgb.g );
  analogWrite(BLUEPIN,  rgb.b );
}

int randomTwinkle() {
  if (twinkleOffset >= 250) {
    twinkleOffset -= 5;
  }
  else if ( twinkleOffset <= 5) {
    twinkleOffset += 5;
  }
  else {
    int delta = random(10);
    if ( delta > 5 ) {
      twinkleOffset += delta - 5;
    }
    else {
      twinkleOffset -= delta;
    }
  }
#ifdef DEBUG
  Serial.print("Twinkle offset: ");
  Serial.println(twinkleOffset);
#endif
  return twinkleOffset;
}

void printHelpMessage() {
  printCurrentSettings();
  Serial.println("");
  Serial.println("Enter \"1\" to set time.");
  Serial.println("Enter \"2\" to set wake alarm");
  Serial.println("Enter \"3\" to set sleep alarm");
  Serial.println("Enter \"4\" to set mode to sleep");
  Serial.println("Enter \"5\" to set mode to awake");
  Serial.println("");
  Serial.println("!!! turn off all line endings !!!");
}

void printCurrentTime() {
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
  if (m < 10) {
    Serial.print("0");
    Serial.print(m);
  }
  else {
    Serial.print(m);
  }
  Serial.print(":");
  if (s < 10 ) {
    Serial.print("0");
    Serial.println(s);
  }
  else {
    Serial.println(s);
  }
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

void printAlarmTime(int hourAddress, int minuteAddress) {
  int alarmMinute = EEPROM.read(minuteAddress);
  Serial.print(EEPROM.read(hourAddress));
  Serial.print(":");
  if (alarmMinute < 10) {
    Serial.print("0");
    Serial.println(alarmMinute);
  }
  else {
    Serial.println(alarmMinute);
  }
}

void printCurrentSettings() {
  printCurrentTime();
  Serial.print("Current wake alarm time ");
  printAlarmTime(1, 0);
  Serial.print("Current sleep alarm time: ");
  printAlarmTime(3, 2);
}

void userSetWakeAlarm() {
  Serial.print("Current alarm time ");
  printAlarmTime(1, 0);
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
  Serial.print("Wake alarm set to ");
  printAlarmTime(1, 0);
  Serial.println(" ");
}

void userSetSleepAlarm() {
  Serial.print("Current alarm time: ");
  printAlarmTime(3, 2);
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
  Serial.print("Sleep alarm set to ");
  printAlarmTime(3, 2);
  Serial.println(" ");
}

void wakeAlarm() {
  colorOn = true;
  int wakeMinuteOfDay = dstOffset(true);
  int wakeMinute = wakeMinuteOfDay % 60;
  int wakeHour = wakeMinuteOfDay / 60;
  rtc.update();
  rtc.setAlarm1(0, wakeMinute, wakeHour);
  Serial.println("Wake");
}

void sleepAlarm() {
  colorOn = false;

  int sleepMinuteOfDay = dstOffset(false);
  int sleepMinute = sleepMinuteOfDay % 60;
  int sleepHour = sleepMinuteOfDay / 60;

  rtc.update();
  rtc.setAlarm2(sleepMinute, sleepHour);
  Serial.println("Sleep");
}

void setStartupState() {
  rtc.update();
  int currentMinuteOfDay = rtc.hour() * 60 + rtc.minute();
  int wakeMinuteOfDay = dstOffset(true);
  int sleepMinuteOfDay = dstOffset(false);

#ifdef DEBUG
  Serial.println("startup numbers: ");
  Serial.print("currentMinuteOfDay ");
  Serial.println(currentMinuteOfDay);
  Serial.print("wakeMinuteOfDay ");
  Serial.println(wakeMinuteOfDay);
  Serial.print("sleepMinuteOfDay ");
  Serial.println(sleepMinuteOfDay);
#endif

  Serial.print("Startup state: ");

  if (currentMinuteOfDay >= sleepMinuteOfDay) {
    sleepAlarm();
  }
  else if (currentMinuteOfDay >= wakeMinuteOfDay) {
    wakeAlarm();
  }
  else {
    sleepAlarm();
  }
}

int dstOffset(bool wake) {
  int memHour, memMin;

  if (wake) {
    memMin = EEPROM.read(0);
    memHour = EEPROM.read(1);
  }
  else {
    memMin = EEPROM.read(2);
    memHour = EEPROM.read(3);
  }

  int minuteOfDay = (memHour * 60 + memMin);

  if (rtc.month() == 3) {
    minuteOfDay += (rtc.day() * 2);
  }
  else if ( rtc.month() > 3 && rtc.month() < 11) {
    minuteOfDay += 60;
  }
  else if (rtc.month() == 11) {
    minuteOfDay += (60 - rtc.day() * 2);
  }

  return minuteOfDay;
}
