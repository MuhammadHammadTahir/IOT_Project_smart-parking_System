// ==== IR Sensor Pins for 6 Slots ====
#define NUM_SLOTS 6
const int irPins[NUM_SLOTS] = {27, 26, 25, 33, 32, 35};  // Example GPIOs for IR sensors

// ==== RGB LED Pins for Slots (Optional Enhancement) ====
//write the remaing pins
const int redPins[NUM_SLOTS]   = {15, 4};
const int greenPins[NUM_SLOTS] = {21, 19};

// ==== Slot Status ====
bool slotOccupied[NUM_SLOTS] = {false, false, false, false, false, false};
int occupiedSlots = 0;


void setup() {
  // Start serial communication
  Serial.begin(115200);

  for(int i=0; i < NUM_SLOTS; i++){
    pinMode(irPins[i], INPUT);
    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
  }
}

void loop() {
  occupiedSlots = 0;
  // Read IR sensor
  for(int i=0; i < NUM_SLOTS; i++){
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
        digitalWrite(redPins[i], LOW);   // Turn ON red LED
        digitalWrite(greenPins[i], HIGH);  // Turn OFF green LED
      }

  }

  
  delay(500); // Wait for half a second
}