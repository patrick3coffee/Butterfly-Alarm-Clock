#include <SPI.h>
#include <SparkFunDS3234RTC.h>
int fadeDelay = 5;

void setup() {
  Serial.begin(9600);
  rtc.begin(10);
  pinMode(3, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(A3, INPUT);
}

void loop() {
  int LEDs[3] = {3, 6, 9};
  for(int LED = 0; LED < 3; LED++){
    for( int val = 0; val < 256; val++){
      analogWrite(LEDs[LED], val);
      delay(fadeDelay);
    }
    for( int val = 255; val >= 0; val--){
      analogWrite(LEDs[LED], val);
      delay(fadeDelay);
    }
  }
}
