#include <SPI.h>
#include <SparkFunDS3234RTC.h>

#define REDPIN 6
#define BLUEPIN 3
#define GREENPIN 9
//#define DEBUG


int R,G,B, brightness;

void setup() {
  Serial.begin(9600);
  rtc.begin(10);
  pinMode(3, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(A3, INPUT_PULLUP);

  Serial.println("Enter \"?\" to print help message");
  R = 0;
  B = 0;
  G = 0;
}

void loop() {
  updateBrightness();
  if (Serial.available()){
    menuSelect();
  }

  if (rtc.alarm1()){
    wakeAlarm();
  }
  else if(rtc.alarm2()){
    sleepAlarm();
  }
  else{
    delay(50);
  }
  
}

void menuSelect(){
  byte menuSelection = Serial.read();
  switch (menuSelection){
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

void updateBrightness(){
  //read brightness from room
  int reading = analogRead(A3);
  brightness = map(reading, 690, 1000, 1, 255);
  
  //translate values to brightness
  int red = map(R, 0, 255, 0, brightness);
  int green = map(G, 0, 255, 0, brightness);
  int blue = map(B, 0, 255, 0, brightness);

  //apply brightness to LEDs
  analogWrite(REDPIN, red);
  analogWrite(BLUEPIN, blue);
  analogWrite(GREENPIN, green);

#ifdef DEBUG
  Serial.print("Brightness: ");
  Serial.println(brightness);
#endif
}

void setWingColor(String color){
  if( color == "red"){
      R = 255;
      G = 0;
      B = 0;
  }
  else if( color == "pink"){
      R = 255;
      G = 100;
      B = 100;
  }
  else if( color == "orange"){
      R = 255;
      G = 65;
      B = 0;
  }
  else if( color == "yellow"){
      R = 255;
      G = 125;
      B = 0;
  }
  else if( color == "green"){
      R = 0;
      G = 255;
      B = 0;
  }
  else if( color == "teal"){
      R = 0;
      G = 100;
      B = 255;
  }
  else if( color == "blue"){
      R = 0;
      G = 0;
      B = 255;
  }
  else if( color == "black"){
      R = 0;
      G = 0;
      B = 0;
  }
  else { //"purple"
      R = 100;
      G = 0;
      B = 255;
  }
  
}

void printHelpMessage(){
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

void printTime(){
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

void userSetTime(){
  Serial.println("Enter current hour (24 hour format):");
  while(!Serial.available()){
    delay(10);
  }
  int newHour = Serial.parseInt();
  Serial.println("Enter current minute:");
  while(!Serial.available()){
    delay(10);
  }
  int newMinute = Serial.parseInt();
  rtc.setTime(0, newMinute, newHour, 0,0,0,0);
  Serial.println("New time set");
}

void userSetWakeAlarm(){
  Serial.println("Enter wake hour (24 hour format):");
  while(!Serial.available()){
    delay(10);
  }
  int newHour = Serial.parseInt();
  Serial.println("Enter current minute:");
  while(!Serial.available()){
    delay(10);
  }
  int newMinute = Serial.parseInt();
  rtc.setAlarm1(0, newMinute, newHour);
  Serial.println("Wake alarm set");
}

void userSetSleepAlarm(){
  Serial.println("Enter sleep hour (24 hour format):");
  while(!Serial.available()){
    delay(10);
  }
  int newHour = Serial.parseInt();
  Serial.println("Enter current minute:");
  while(!Serial.available()){
    delay(10);
  }
  int newMinute = Serial.parseInt();
  rtc.setAlarm2(newMinute, newHour);
  Serial.println("Sleep alarm set");
}

void wakeAlarm(){
  setWingColor("teal");
}

void sleepAlarm(){
  setWingColor("black");
}

