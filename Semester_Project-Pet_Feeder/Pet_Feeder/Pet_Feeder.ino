// Necessary Packages
#include <Ultrasonic.h>
#include <math.h>
#include <Grove_LED_Bar.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <LWiFi.h>

// WiFi Related
char ssid[] = "your-WiFi-SSID";
char pass[] = "your-WiFi-Password";
int status = WL_IDLE_STATUS;
WiFiClient client;

// ThingSpeak Related
char server[] = "api.ThingSpeak.com";
String writeAPIKey = "your-ThingSpeak-APIKEY";

// Constants and Object Declarations
#define LIGHT_SENSOR_PIN A0
Ultrasonic ultrasonic(2);
Grove_LED_Bar bar(5, 4, 0);
Servo myservo;
LiquidCrystal_I2C lcd(0x27);

// Global Variables
int pos = 0; // Initial servo position
const int MAX_HEIGHT = 5; // Feeder height in cm
const int threshold = 300; // Light sensor threshold
int previousReading = 0; // Previous light sensor reading
unsigned long messageStartTime = 0;
const unsigned long displayDuration = 5000; // LCD display duration in ms
bool isMessageDisplayed = false; // Message display status

// Feeding Time
unsigned long previousFeedMillis = 0;
const unsigned long feedInterval = 30000; // Feed every 30 seconds (testing)
bool isFeeding = false; // Feeding status

// Water Status
unsigned long waterCheckStart = 0;
const unsigned long waterCheckInterval = 120000; // Check water level every 2 mins
int lastWaterLevel = -1;
bool waterChanged = false;

// Feeder Level Data for ThingSpeak
int feederLevelData = 0;

// Time Control (to prevent too frequent uploads)
unsigned long lastUpdateTS = 0;
const unsigned long updateInterval = 20000; // Upload every 20 seconds

void setup() {
    Serial.begin(9600); // Initialize serial communication
    bar.begin(); // Initialize LED bar
    lcd.begin(16, 2); // Initialize LCD display
    lcd.backlight(); // Turn on LCD backlight
    lcd.setCursor(0, 0);
    lcd.print("Awaiting Input..."); // Display initial message
    myservo.attach(11); // Attach servo to pin 11

    // Connect to WiFi
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
        delay(1000);
    }
    Serial.println("Connected to WiFi!");
    printWifiStatus(); // Display WiFi status
}

void loop() {
    detectMealTime(); // Check if pet is eating
    motorControl(); // Control feeding mechanism
    detectFoodLevel(); // Monitor food level
    detectWaterStatus(); // Monitor water level

    // Upload data every 20 seconds
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTS >= updateInterval) {
        lastUpdateTS = currentMillis;
        sendDataToThingSpeak(); // Upload feeder percentage
    }
}

void detectMealTime() {
    int currentReading = analogRead(LIGHT_SENSOR_PIN); // Read light sensor
    int lightReadingDiff = abs(currentReading - previousReading); // Calculate difference

    if (lightReadingDiff > threshold && !isMessageDisplayed) {
        isMessageDisplayed = true;
        messageStartTime = millis();
        lcd.setCursor(0, 0);
        lcd.print("Pet has eaten!   "); // Display eating message
        Serial.println("Pet has eaten!");
    }

    if (isMessageDisplayed && millis() - messageStartTime >= displayDuration) {
        lcd.setCursor(0, 0);
        lcd.print(""); // Clear LCD line
        isMessageDisplayed = false;
    }
    previousReading = currentReading; // Update previous reading
}

void motorControl() {
    unsigned long currentMillis = millis();

    if ((currentMillis - previousFeedMillis) >= feedInterval) {
        previousFeedMillis = currentMillis;
        int currentLight = analogRead(LIGHT_SENSOR_PIN); // Read light sensor

        if (currentLight > 800) { // If light level is high, initiate feeding
            isFeeding = true;
            for (pos = 0; pos <= 65; pos++) { // Move servo to feed
                lcd.setCursor(0, 1);
                lcd.print("Feeding...");
                myservo.write(pos);
            }
        }
    }

    if (isFeeding) {
        int currentLight = analogRead(LIGHT_SENSOR_PIN); // Read light sensor
        if (currentLight <= 800) { // If light level drops, end feeding
            for (pos = 65; pos >= 0; pos--) { // Move servo back
                myservo.write(pos);
                lcd.setCursor(0, 1);
                lcd.print("End Feeding...");
            }
            isFeeding = false;
        }
    }
}

void detectFoodLevel() {
    long distance = ultrasonic.MeasureInCentimeters(); // Measure food level
    if (distance > 0 && distance <= MAX_HEIGHT) {
        int remainingLevel = MAX_HEIGHT - distance;
        int percentage = (remainingLevel * 100) / MAX_HEIGHT; // Calculate percentage
        ledBarHandler(percentage); // Update LED bar
        feederLevelData = percentage;

        if (percentage < 20) {
            lcd.setCursor(0, 0);
            lcd.print("Refill food needed");
        }
    } else {
        feederLevelData = 0; // Invalid reading
    }
}

void detectWaterStatus() {
    unsigned long currentMillis = millis();
    if (waterCheckStart == 0) {
        waterCheckStart = currentMillis; // Initialize first check
    }

    if ((currentMillis - waterCheckStart) >= waterCheckInterval) {
        waterCheckStart = currentMillis;
        int currentWaterLevel = getWaterLevel(); // Get current water level
        if (lastWaterLevel == -1) {
            lastWaterLevel = currentWaterLevel; // Initialize last water level
        } else if (abs(currentWaterLevel - lastWaterLevel) > 5) { // Significant change
            waterChanged = true;
            Serial.println("Water level changed.");
        }
        lastWaterLevel = currentWaterLevel; // Update last water level
    }
}

void ledBarHandler(int percentage) {
    if (percentage < 10) {
        bar.setBits(0b0000000001);
    } else if (percentage >= 10 && percentage < 20) {
        bar.setBits(0b0000000011);
    } else if (percentage >= 20 && percentage < 30) {
        bar.setBits(0b0000000111);
    } else if (percentage >= 30 && percentage < 40) {
        bar.setBits(0b0000001111);
    } else if (percentage >= 40 && percentage < 50) {
        bar.setBits(0b0000011111);
    } else if (percentage >= 50 && percentage < 60) {
        bar.setBits(0b0000111111);
    } else if (percentage >= 60 && percentage < 70) {
        bar.setBits(0b0001111111);
    } else if (percentage >= 70 && percentage < 80) {
        bar.setBits(0b0011111111);
    } else if (percentage >= 80 && percentage < 90) {
        bar.setBits(0b0111111111);
    } else if (percentage >= 90 && percentage <= 100) {
        bar.setBits(0b1111111111);
    } else {
        bar.setBits(0b0000000000);
    }
}

int getWaterLevel() {
    return random(0, 101); // Simulate water level (replace with actual sensor)
}

void sendDataToThingSpeak() {
    if (client.connect(server, 80)) {
        String getStr = "GET /update?api_key=" + writeAPIKey +
                        "&field1=" + String(feederLevelData) +
                        " HTTP/1.1\r\n" +
                        "Host: api.ThingSpeak.com\r\n" +
                        "Connection: close\r\n\r\n";
        client.print(getStr); // Send request
        delay(1000);
        client.stop(); // Disconnect
    } else {
        Serial.println("Failed to connect to ThingSpeak.");
    }
}

void printWifiStatus() {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
}
