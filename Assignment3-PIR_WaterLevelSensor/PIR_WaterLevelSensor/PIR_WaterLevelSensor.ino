#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int pirPin = 2;
const int buzzerPin = 3;
const int waterSensorPin = A0;
const int waterThreshold = 500;
const int noiseThreshold = 50;
int baseline = 0;

void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  noTone(buzzerPin);
 
  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  for (int i = 0; i < 10; i++) {
    baseline += analogRead(waterSensorPin);
    delay(10);
  }
  baseline /= 10;
}

void loop() {
  int pirState = digitalRead(pirPin);
  int waterLevel = analogRead(waterSensorPin) - baseline;

  if (waterLevel < noiseThreshold) {
    waterLevel = 0;
  }
  
  if (pirState == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("People       ");
    
    if (waterLevel > waterThreshold) {
      lcd.setCursor(0, 0);
      lcd.print("Urgent       ");
      tone(buzzerPin, 500);

    } else {
      lcd.setCursor(0, 0);
      lcd.print("Safety       ");
      noTone(buzzerPin);
    }
    
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Nobody       ");
    lcd.setCursor(0, 0);
    lcd.print("             ");
    digitalWrite(buzzerPin, LOW);
    noTone(buzzerPin);
  }

  Serial.print("PIR State: ");
  Serial.print(pirState == HIGH ? "People" : "Nobody");
  Serial.print(" | Water Level: ");
  Serial.println(waterLevel);
  
  delay(100);
}
