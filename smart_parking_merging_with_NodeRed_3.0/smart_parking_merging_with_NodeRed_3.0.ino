#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <ESP32Servo.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>  
#include <ArduinoJson.h>
const char* ssid = "eman's S24 Ultra";
const char* password = "12345678";
const char* serverUrl = "http://192.168.121.20:1880/slots";


Servo myServo;

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);

#define ir_out 36 

#define NUM_SLOTS 6
const int irPins[NUM_SLOTS] = { 27, 26, 25, 33, 32, 35 };  // Example GPIOs for IR sensors

// ==== RGB LED Pins for Slots (Optional Enhancement) ====
//write the remaing pins
const int redPins[NUM_SLOTS] = { 15, 4, 16, 17, 18, 19 };
const int greenPins[NUM_SLOTS] = { 14, 34, 2, 23, 5, 13 };

// ==== Slot Status ====
bool slotOccupied[NUM_SLOTS] = { false, false, false, false, false, false };
int occupiedSlots = 0;


// Timing for non-blocking servo control
bool servoActive = false;
unsigned long servoActivatedTime = 0;
const unsigned long SERVO_DURATION = 5000;

void setup(void) {
  Serial.begin(115200);
  Serial.println("System initialized");

  for(int i=0; i < NUM_SLOTS; i++){
  pinMode(irPins[i], INPUT);
  pinMode(redPins[i], OUTPUT);
  pinMode(greenPins[i], OUTPUT);
  }
  pinMode(ir_out, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected. IP: " + WiFi.localIP().toString());
  ArduinoOTA.setHostname("ESP-OTA");
  ArduinoOTA.begin();

  Wire.begin(21, 22);

  myServo.attach(12);

  Serial.print("servo is attached");
  
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);  // Choose a font
  nfc.begin();
}

void loop() {
  ArduinoOTA.handle();
  readNFC();
  occupiedSlots = 0;
  // Read IR sensor
  for (int i = 0; i < NUM_SLOTS; i++) {
    slotOccupied[i] = digitalRead(irPins[i]);
    if (slotOccupied[i] == false) {
      Serial.print("Object Detected at sensor ");
      Serial.print(i);
      Serial.println("!");
      occupiedSlots++;
      digitalWrite(redPins[i], HIGH);   // Turn ON red LED
      digitalWrite(greenPins[i], LOW);  // Turn OFF green LED
    } else {
      Serial.print("No Object Detected at sensor ");
      Serial.print(i);
      Serial.println(".");
      digitalWrite(redPins[i], LOW);     // Turn ON red LED
      digitalWrite(greenPins[i], HIGH);  // Turn OFF green LED
    }
  }
  int Total_slots = 6;
  int Avalible_Slots = Total_slots - occupiedSlots;
  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "Welcome to smart Parking");  // Line 1
  u8g2.drawStr(0, 40, "Total Slots : ");
  u8g2.setCursor(100, 40);  // Position to show Total Slots value
  u8g2.print(Total_slots);  // Display the total number of slots

  u8g2.drawStr(0, 60, "Available Slots : ");
  u8g2.setCursor(120, 60);     // Position to show Available Slots value
  u8g2.print(Avalible_Slots);  // Display the available slots

  u8g2.sendBuffer();  // Send the buffer to the display
/*
  if (!digitalRead(ir_out)){
    myServo.write(90);
    delay(5000);
    myServo.write(0);
  }*/

  if (!digitalRead(ir_out) && !servoActive) {
    Serial.println("hello ");
    myServo.write(90);
    servoActivatedTime = millis();
    servoActive = true;
  }

  // Check if servo should return
  if (servoActive && millis() - servoActivatedTime >= SERVO_DURATION) {
    myServo.write(0);
    servoActive = false;
  }

  senddata();
}

/*void readNFC() {
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();
    tagId = tag.getUidString();
    myServo.write(90);


    delay(5000);
    myServo.write(0);
  }
  delay(1000);
}*/

void readNFC() {
  static bool cardPreviouslyDetected = false;
  static unsigned long lastReadTime = 0;
  // Check every 300ms for performance
  if (millis() - lastReadTime < 300) return;
  lastReadTime = millis();
  if (nfc.tagPresent()) {
    if (!cardPreviouslyDetected) {  // Card just placed
      NfcTag tag = nfc.read();      // Blocking, but only called once per placement
      tag.print();
      tagId = tag.getUidString();
      myServo.write(90);
      servoActivatedTime = millis();
      servoActive = true;
      cardPreviouslyDetected = true;  // Mark as handled
    }
  } else {
    cardPreviouslyDetected = false;   // Card removed — allow next read
  }
}

void senddata(){
  // Build JSON
  StaticJsonDocument<200> doc;
  JsonArray slots = doc.createNestedArray("slots");
  for (int i = 0; i < NUM_SLOTS; i++) {
    slots.add(slotOccupied[i] ? 1 : 0);
  }

  String jsonPayload;
  serializeJson(doc, jsonPayload);
  Serial.println("Sending JSON: " + jsonPayload);

  // Send HTTP POST
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("POST success: " + String(httpResponseCode));
    } else {
      Serial.println("POST failed, error: " + http.errorToString(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}