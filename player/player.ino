#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Fri3dMatrix.h>
#include <Fri3dAccelerometer.h>
#include <Fri3dBuzzer.h>
#include <Fri3dButtons.h>

const String teamTag = "R"; // choose from R|G|B
uint64_t playerId;
String playerTag;

Fri3dMatrix matrix = Fri3dMatrix();
Fri3dButtons buttons = Fri3dButtons();
Fri3dAccelerometer accel = Fri3dAccelerometer();

#define SERVICE_UUID        "111fc111-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "11154111-36e1-4688-b7f5-ea07361b26a8"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

enum State { STATE_DEAD, STATE_CONQUER, STATE_CONQUERING, STATE_FIGHTING, STATE_BOMB, STATE_BOMBING, STATE_UNDERBOMBING };
State state = STATE_FIGHTING; // TODO: change to start as dead player

// fightmode vars
long fight_startTime = 0; // millis to capture start of fightmode
const long fight_maxDurationTime = 20; // time in seconds
long fight_remainingTime = fight_maxDurationTime * 1000;
float fight_startAccel[3];

void setup() {
  Serial.begin(9600);

  // player details
  Serial.println("===== starting player... =====");
  Serial.print("Player team: ");
  Serial.println(teamTag);
  playerId = ESP.getEfuseMac();
  Serial.print("Player id: ");
  Serial.println(String((uint32_t)playerId));
  playerTag = teamTag + String((uint32_t)playerId);
  Serial.print("Player BLE tag: ");
  Serial.println(playerTag);

  // initialise ble
  BLEDevice::init(playerTag.c_str());

  // user interactions
  buttons.setReleasedCallback(0, startFightMode); // button 0 starts fightmode
  // TODO buttons.setReleasedCallback(1, start);
  buttons.startDebounceThread();
}

void loop() {
  switch (state) {
    case STATE_DEAD:
      if (Serial.available() > 0) {   
        int inByte = Serial.read();
        state = STATE_CONQUER;
        Serial.println("revived");
      }
      break;
    case STATE_FIGHTING:
      sensors_event_t event; 
      accel.getEvent(&event);
      if (millis() > (fight_startTime + 1000)) {
        fight_startTime = millis();
        fight_remainingTime = fight_remainingTime - 1000;
        //Serial.print("remaining time: ");
        //Serial.println(fight_remainingTime);
        drawFightingDisplay();
        Serial.print("X-acc: ");
        Serial.println(event.acceleration.x);
        Serial.print("Y-acc: ");
        Serial.println(event.acceleration.y);
        Serial.print("Z-acc: ");
        Serial.println(event.acceleration.z);
      }
      if (fight_remainingTime <= 0) {
        state = STATE_CONQUER;
      }
      break;
    case STATE_BOMB:

      break;
    case STATE_CONQUER:
      startConquerMode();
      state = STATE_CONQUERING;
      break;
  }
  delay(10);
}

void drawBasicDisplay(String gameMode = "C") {
  matrix.clear();
  matrix.drawString(2, teamTag);
  matrix.drawString(9, gameMode);
}

void drawFightingDisplay() {
  matrix.clear();
  int remaining = fight_remainingTime / 1000;
  if (remaining < 10) {
    matrix.drawString(9, String(remaining));
  } else {
    int seconds = remaining % 10;
    remaining = remaining / 10;
    matrix.drawString(3, String(remaining));
    matrix.drawString(9, String(seconds));
  }
  
}

// beacon mode, enables players to conquer towers
void startConquerMode() {
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  pCharacteristic->setValue("conquering ...");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("conquerer active ...");
  drawBasicDisplay("C");
}

void startFightMode() {
  drawBasicDisplay("F");
  sensors_event_t event; 
  accel.getEvent(&event);
  fight_startAccel[0] = event.acceleration.x;
  fight_startAccel[1] = event.acceleration.y;
  fight_startAccel[2] = event.acceleration.z;
  fight_startTime = millis();
  state = STATE_FIGHTING;
}
