#include <Wire.h>
#include <SeeedOLED.h>
#include <LBLE.h>
#include <LBLEPeriphral.h>

LBLEService ledService("19B10010-E8F2-537E-4F6C-D104768A1214");
LBLECharacteristicInt switchCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", LBLE_READ | LBLE_WRITE);

int value = -1;
unsigned long previousMillis = 0;
unsigned long blinkInterval = 0;
bool ledState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Wire.begin();
  SeeedOled.init();
  SeeedOled.deactivateScroll();
  SeeedOled.setPageMode();

  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }
  Serial.println("BLE ready");

  LBLEAdvertisementData advertisement;
  advertisement.configAsConnectableDevice("Hello World");

  LBLEPeripheral.setName("Hello World");
  ledService.addAttribute(switchCharacteristic);
  LBLEPeripheral.addService(ledService);
  LBLEPeripheral.begin();
  LBLEPeripheral.advertise(advertisement);
}

void loop() {

  if (switchCharacteristic.isWritten()) {
    value = switchCharacteristic.getValue();
    Serial.print("BLE value written: ");
    Serial.println(value);

    switch (value) {
      case 0:
        digitalWrite(LED_BUILTIN, LOW);
        ledState = LOW;
        blinkInterval = 0;
        displayMessage("OFF", "");
        break;
      case 1:
        blinkInterval = 5000;
        displayMessage("Blink", "5 sec");
        break;
      case 2:
        blinkInterval = 3000;
        displayMessage("Blink", "3 sec");
        break;
      case 3:
        blinkInterval = 1000;
        displayMessage("Blink", "1 sec");
        break;
      case 4:
        digitalWrite(LED_BUILTIN, HIGH);
        ledState = HIGH;
        blinkInterval = 0;
        displayMessage("ON", "");
        break;
      default:
        displayMessage("Unknown", "Command");
        break;
    }
  }

  if (blinkInterval > 0) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
    }
  }
}

void displayMessage(const char* line1, const char* line2) {
  SeeedOled.clearDisplay();
  SeeedOled.setTextXY(0, 0);
  SeeedOled.putString(line1);
  SeeedOled.setTextXY(1, 0);
  SeeedOled.putString(line2);
}
