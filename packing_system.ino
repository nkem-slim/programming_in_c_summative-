#include <Arduino.h>
// C++ code
//


const uint8_t MAX_SPACES = 4;
const uint8_t LED_PINS[MAX_SPACES] = {2, 3, 4, 5};
const uint8_t ENTRY_BTN = 6;
const uint8_t EXIT_BTN  = 7;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long STATUS_INTERVAL = 3000;

struct ParkingSpace {
  uint8_t id;
  bool occupied;
  unsigned long lastChange;
};

ParkingSpace *spaces = nullptr;
uint8_t occupiedCount = 0;

bool lastEntryState = HIGH;
bool lastExitState = HIGH;
unsigned long lastEntryDebounce = 0;
unsigned long lastExitDebounce = 0;

unsigned long lastStatusPrint = 0;

void initParkingSystem();
void freeParkingSystem();
void updateLEDs();
bool tryPark();
bool tryLeave();
void printStatus();
void handleButtons();

void setup() 
{
  Serial.begin(9600);
  while(!Serial) {;} // used to wait for Serial
  
  // Pins
  for(uint8_t i=0; i<MAX_SPACES; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  
  pinMode(ENTRY_BTN, INPUT_PULLUP);
  pinMode(EXIT_BTN, INPUT_PULLUP);
  
  initParkingSystem();
  printStatus();
  Serial.println(F("System ready. Press ENTRY or EXIT buttons."));
}


void loop()
{
  handleButtons();
  
  if(millis() - lastStatusPrint >= STATUS_INTERVAL) {
    printStatus();
    lastStatusPrint = millis();
  }
}

void initParkingSystem()
{
  spaces = new ParkingSpace[MAX_SPACES];
  if(spaces == nullptr) {
    Serial.println(F("Error occured: Memory allocation failed!"));
    while(1); //halt
  }
  
  for(uint8_t i = 0; i<MAX_SPACES; i++) {
    spaces[i].id = i + 1;
    spaces[i].occupied = false;
    spaces[i].lastChange = 0;
  }
  
  occupiedCount = 0;
  updateLEDs();
}

void freeParkingSystem()
{
  if(spaces != nullptr){
    delete[] spaces;
    spaces = nullptr;
  }
}

void updateLEDs()
{
  for(uint8_t i = 0; i<MAX_SPACES; i++) {
    ParkingSpace *p = spaces + i;
    digitalWrite(LED_PINS[i], p->occupied ? HIGH : LOW);
  }
}

bool tryPark()
{
  if(occupiedCount >= MAX_SPACES) {
    Serial.println("Cannot: Parking is full. Cannot enter.");
    return false;
  }
  
  for(uint8_t i = 0; i<MAX_SPACES; i++) {
    ParkingSpace *p = &spaces[i];
    if(!p->occupied) {
      p->occupied = true;
      p->lastChange = millis();
      occupiedCount++;
      updateLEDs();
      Serial.print(F("Vehicle entered space "));
      Serial.println(p->id);
      return true;
    }
  }
  
  return false;
}

bool tryLeave()
{
  if(occupiedCount == 0){
    Serial.println(F("Error: no vehicle(s) to exit."));
    return false;
  }
  
  for(int i=MAX_SPACES - 1; i>= 0; i--) {
    ParkingSpace *p = &spaces[i];
    if(p->occupied) {
      p->occupied = false;
      p->lastChange = millis();
      occupiedCount--;
      updateLEDs();
      Serial.print(F("Vehicle left space "));
      Serial.println(p->id);
      return true;
    }
  }
  
  return false;
}

void handleButtons()
{
  // Simple version for Tinkercad
  if (digitalRead(ENTRY_BTN) == LOW) {
    tryPark();
    delay(300); // prevent multiple triggers
  }

  if (digitalRead(EXIT_BTN) == LOW) {
    tryLeave();
    delay(300);
  }
}

void printStatus()
{
  Serial.println(F("============ Parking Status ============"));
  Serial.print(F("Occupied : "));
  Serial.println(occupiedCount);
  Serial.print(F("Available: "));
  Serial.println(MAX_SPACES - occupiedCount);
  
  for(uint8_t i = 0;i<MAX_SPACES; i++) {
    ParkingSpace *p = spaces + i;
    Serial.print(F("Space "));
    Serial.print(p->id);
    Serial.print(F(": "));
    Serial.println(p->occupied ? F("Occupied") : F("Free"));
  }
  
  Serial.println(F("======================================="));
}


































































































