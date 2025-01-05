#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int RED_LED = 6;
const int YELLOW_LED = 7;
const int GREEN_LED = 8;
const int BUZZER = 9;

const int BUZZER_FREQ = 1000;
const int BUZZER_DURATION = 200;
const int BUZZER_REPEAT = 5;
const int YELLOW_INTERVAL = 1000;
const int GREEN_INTERVAL = 3000;

volatile bool keepBuzzing = false;

class DeviceController {
  int ledPin;
  bool buzzerState;

public:
  DeviceController() : ledPin(-1), buzzerState(false) {}

  DeviceController& setLED(int pin) {
    this->ledPin = pin;
    return *this;
  }

  DeviceController& ledOn() {
    if (ledPin != -1) {
      digitalWrite(ledPin, HIGH);
    }
    return *this;
  }

  DeviceController& ledOff() {
    if (ledPin != -1) {
      digitalWrite(ledPin, LOW);
    }
    return *this;
  }

  DeviceController& displayMessage(const char* msg) {
    lcd.clear();
    lcd.print(msg);
    return *this;
  }

  DeviceController& buzzerOn(int freq, int duration) {
    tone(BUZZER, freq, duration);
    return *this;
  }

  DeviceController& buzzerOff() {
    noTone(BUZZER);
    return *this;
  }

  DeviceController& buzzerRepeat(int freq, int duration, int repeat, int interval) {
    for (int i = 0; i < repeat; i++) {
      buzzerOn(freq, duration);
      delay(duration);
      buzzerOff();
      delay(interval);
    }
    return *this;
  }
};

DeviceController device;

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);
  lcd.init();          
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Awaiting Input...");
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '\n' || input == '\r') {
      return;
    }
    inputHandler(input);
  }

  if (keepBuzzing) {
    tone(BUZZER, BUZZER_FREQ);
  }
}

void resetOutputs() {
  lcd.clear();
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  keepBuzzing = false;
  device.buzzerOff();
}

void initialMessage(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Awaiting Input...");
}

void inputHandler(char input) {
  resetOutputs();
  input = toupper(input);
  Serial.println("=== Session Started ===");

  switch (input) {
    case 'C':
      break;
    case 'R':
      device.setLED(RED_LED)
            .ledOn()
            .displayMessage("RED ON");
      keepBuzzing = true;
      break;
    case 'Y':
      device.setLED(YELLOW_LED)
            .ledOn()
            .displayMessage("YELLOW ON")
            .buzzerRepeat(BUZZER_FREQ, BUZZER_DURATION, BUZZER_REPEAT, YELLOW_INTERVAL);
      initialMessage();
      break;
    case 'G':
      device.setLED(GREEN_LED)
            .ledOn()
            .displayMessage("GREEN ON")
            .buzzerRepeat(BUZZER_FREQ, BUZZER_DURATION, BUZZER_REPEAT, GREEN_INTERVAL);
      initialMessage();
      break;
    default:
      device.displayMessage("(422) Unprocessable Entity");
      Serial.println("Invalid command received.");
      delay(3000);
      initialMessage();
      break;
  }
}
