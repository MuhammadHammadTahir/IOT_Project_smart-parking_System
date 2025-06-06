#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <ESP32Servo.h>
#include <U8g2lib.h>


Servo myServo;

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

#define NUM_SLOTS 6
const int irPins[NUM_SLOTS] = { 27, 26, 25, 33, 32, 35 };  // Example GPIOs for IR sensors

// ==== RGB LED Pins for Slots (Optional Enhancement) ====
//write the remaing pins
const int redPins[NUM_SLOTS] = { 15, 4, 16, 17, 18, 19 };
const int greenPins[NUM_SLOTS] = { 14, 34, 2, 24, 5, 13 };

// ==== Slot Status ====
bool slotOccupied[NUM_SLOTS] = { false, false, false, false, false, false };
int occupiedSlots = 0;

void setup(void) {
  Serial.begin(115200);
  Serial.println("System initialized");
  myServo.attach(12);
  nfc.begin();
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);  // Choose a font
                                       //u8g2.drawStr(0, 20, "Ehman ap galat");         // Line 1
                                       //u8g2.drawStr(0, 40, "library use kar rahi thi"); // Line 2
                                       //u8g2.sendBuffer(); // Send to display

  for(int i=0; i < NUM_SLOTS; i++){
  pinMode(irPins[i], INPUT);
  pinMode(redPins[i], OUTPUT);
  pinMode(greenPins[i], OUTPUT);
}
}

void loop() {
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
}

void readNFC() {
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();
    tagId = tag.getUidString();
    myServo.write(90);


    delay(5000);
    myServo.write(0);
  }
  delay(1000);
}