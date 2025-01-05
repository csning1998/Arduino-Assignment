#include "Ultrasonic.h"
#include <Grove_LED_Bar.h>

Ultrasonic ultrasonic(2);
Grove_LED_Bar bar(4, 3, 0);

const int BUZZER = 9;
const int freq = 1000;
const int duration = 200;
const int repeatInterval = 250;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  bar.begin();
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  long range;

  range = ultrasonic.MeasureInCentimeters();
  Serial.print("Distance is ");
  Serial.print(range); // 0~400cm
  Serial.println(" cm");

  ledBuzzerHandler(range);
  
  delay(100);
}

void ledBuzzerHandler(long range) {
  if (range < 5) {
    tone(BUZZER, freq, duration);
    bar.setBits(0b1111111111);
  } else if (range >= 5 && range < 10) {
    nonBlockingBuzz();
    bar.setBits(0b0011111111);
  } else if (range >= 10 && range < 15) {
    nonBlockingBuzz();
    bar.setBits(0b0000111111);
  } else if (range >= 15 && range < 20) {
    nonBlockingBuzz();
    bar.setBits(0b0000001111);
  } else if (range >= 20 && range < 25) {
    nonBlockingBuzz();
    bar.setBits(0b0000000011);
  } else {
    bar.setBits(0b0000000000);
    noTone(BUZZER);
  }
}

void nonBlockingBuzz() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= repeatInterval) {
    previousMillis = currentMillis;
    tone(BUZZER, freq, duration);
  }
}
