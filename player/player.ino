#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>

#include <Fri3dMatrix.h>
//#include <Fri3dButtons.h>
#include <Fri3dAccelerometer.h>
#include <Fri3dBuzzer.h>

const String teamTag = "R"; // choose from R|G|B
uint16_t teamId;
uint64_t playerId;
String playerTag;

Fri3dMatrix matrix = Fri3dMatrix();
//Fri3dButtons buttons = Fri3dButtons();
Fri3dAccelerometer accel = Fri3dAccelerometer();
Fri3dBuzzer buzzer = Fri3dBuzzer();

#define SERVICE_UUID        "111fc111-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "11154111-36e1-4688-b7f5-ea07361b26a8"
#define BEACON_UUID         "8ec76ea3-6668-48da-9866-75be8bc86f4d"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;

enum State { STATE_CONQUER, STATE_CONQUERING, STATE_BOMB, STATE_BOMBING, STATE_UNDERBOMBING, 
             STATE_FIGHT, STATE_FIGHTING, STATE_DYING, STATE_DEAD };
State state = STATE_CONQUER;

// fightmode vars
long fight_startTime = 0; // millis to capture start of fightmode
const long fight_maxDurationTime = 5; // time in seconds
long fight_remainingTime;
float fight_startAccel[3];
float fight_maxDeviation = 2.00;

// bombingmode vars
long bomb_startTime = 0; // millis to capture start of bombmode
const long bomb_durationTime = 5; // time in seconds TODO make larger
long bomb_remainingTime;

// ui
// I know there is a button class, but this gave errors in multi_heap.c, 
// see https://github.com/Fri3dCamp/Fri3dBadge/issues/4
#define BUTTONBOOT_PIN 0
#define BUTTON0_PIN 36
#define BUTTON1_PIN 39
#define TOUCH0_PIN 12
#define TOUCH1_PIN 14
int buttons[2] = {BUTTON0_PIN, BUTTON1_PIN};
int buttonPressed[2] = {0, 0};

void setup() {
  Serial.begin(9600);

  // player details
  setTeamId();
  Serial.println("===== starting player... =====");
  Serial.print("Player team: ");
  Serial.println(teamTag);
  playerId = ESP.getEfuseMac();
  Serial.print("Player id: ");
  Serial.println(String((uint32_t)playerId));
  playerTag = teamTag + String((uint32_t)playerId);
  Serial.print("Player BLE tag: ");
  Serial.println(playerTag);

  Serial.print("default state: ");
  Serial.println(state);

  // initialise ble
  BLEDevice::init(playerTag.c_str());
  
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, 
                                                   BLECharacteristic::PROPERTY_READ
                                                   |BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue(playerTag.c_str());
  pService->start();  
  pAdvertising = pServer->getAdvertising();

  // user interactions
  pinMode( BUTTON0_PIN, INPUT_PULLUP );
  pinMode( BUTTON1_PIN, INPUT_PULLUP );

  // started
  playDeadSound();
}

void loop() {
  if (checkButtonReleased(0)) {
    startFightMode();
  } else if (checkButtonReleased(1)) {
    startBombing();
  }
  
  switch (state) {
    case STATE_DYING:
        startDead();
      break;
    case STATE_DEAD:
      // revive method
      if (Serial.available() > 0) {   
        int inByte = Serial.read();
        Serial.println(inByte, DEC);
        if (inByte == 108) { // 'l'
          state = STATE_CONQUER;
          playAliveSound();
          Serial.println("revived");
        }
      }
      break;
    case STATE_FIGHT:
      startFightMode();
      break;
    case STATE_FIGHTING:
      sensors_event_t event; 
      accel.getEvent(&event);

      if ((abs(fight_startAccel[0] - event.acceleration.x) > fight_maxDeviation) || 
          (abs(fight_startAccel[1] - event.acceleration.y) > fight_maxDeviation) || 
          (abs(fight_startAccel[2] - event.acceleration.z) > fight_maxDeviation)) {
        playDeadSound();
      }

      if (millis() > (fight_startTime + 1000)) {
        fight_startTime = millis();
        fight_remainingTime = fight_remainingTime - 1000;
        drawFightingDisplay(fight_remainingTime);
      }
      if (fight_remainingTime <= 0) {
        state = STATE_CONQUER;
      }
      break;
    case STATE_BOMB:
      startBombing();
      break;
    case STATE_BOMBING:
      if (millis() > (bomb_startTime + 1000)) {
        bomb_startTime = millis();
        bomb_remainingTime = bomb_remainingTime - 1000;
        drawFightingDisplay(bomb_remainingTime);
        playBombingSound();
      }
      if (bomb_remainingTime <= 1) {
        playExplodingSound();
        // TODO send last beaconsignal
        state = STATE_DYING;
      }
      break;
    case STATE_CONQUER:
      startConquerMode();
      break;
    case STATE_CONQUERING:
      // TODO checking for incoming bombing
      break;
  }
 
  delay(10);
}

void setTeamId() {
  if (teamTag == "R") {
    teamId = 1;
  } else if (teamTag == "G") {
    teamId = 2;
  } else if (teamTag == "B") {
    teamId = 3;
  }
}

bool checkButtonReleased(int button) {
  int currentValue = digitalRead(buttons[button]);
  if (currentValue == LOW) {
    buttonPressed[button] = 1;
  }
  if ((currentValue == HIGH) && (buttonPressed[button] == 1)) {
    buttonPressed[button] = 0;
    return true;
  }
  return false;
}

void drawBasicDisplay(String gameMode = "C") {
  matrix.clear();
  matrix.drawString(2, teamTag);
  matrix.drawString(9, gameMode);
}

void drawFightingDisplay(long fight_remainingTime) {
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

void startConquerMode() {
  if ((state != STATE_DEAD) && (state != STATE_FIGHTING)) {
    Serial.println("starting conquering ...");
    state = STATE_CONQUERING;

    // change ble mode
    pAdvertising->stop();
    setBeacon();
    
    drawBasicDisplay("C");
  }
}

void startFightMode() {
  if ((state != STATE_DEAD) && (state != STATE_FIGHTING)) {
    Serial.println("starting fighting ...");
    state = STATE_FIGHTING;

    // change ble mode
    pAdvertising->stop();

    // timeout for fighting
    matrix.clear();
    matrix.setPixel( 3, 0, 1 );
    matrix.setPixel( 2, 1, 1 );
    matrix.setPixel( 4, 1, 1 );
    matrix.setPixel( 1, 2, 1 );
    matrix.setPixel( 5, 2, 1 );
    matrix.setPixel( 2, 3, 1 );
    matrix.setPixel( 4, 3, 1 );
    matrix.setPixel( 3, 4, 1 );

    delay(1000); // TODO improve, make blinking stuff
    
    // benchmark accelerator
    sensors_event_t event; 
    accel.getEvent(&event);
    fight_startAccel[0] = event.acceleration.x;
    fight_startAccel[1] = event.acceleration.y;
    fight_startAccel[2] = event.acceleration.z;

    // init timeout
    fight_startTime = millis();
    fight_remainingTime = fight_maxDurationTime * 1000;

    drawBasicDisplay("F");
  }
}

void startBombing() {
  if ((state != STATE_DEAD) && (state != STATE_FIGHTING)) {
    Serial.println("starting bombing ...");
    state = STATE_BOMBING; 
    
    // change ble mode
    pAdvertising->stop();
    setBeacon();

    // init timeout
    bomb_startTime = millis();
    bomb_remainingTime = bomb_durationTime * 1000;

    drawBasicDisplay("B");
  }
}

void startDead() {
  if (state != STATE_DEAD) {
    Serial.println("starting dead ...");
    state = STATE_DEAD; 
    
    // change ble mode
    pAdvertising->stop();

    playDeadSound();
    
    // showing dead-mask
    matrix.clear();
    matrix.setPixel( 3, 2, 1 );
    matrix.setPixel( 2, 3, 1 );
    matrix.setPixel( 3, 3, 1 );
    matrix.setPixel( 4, 3, 1 );
    matrix.setPixel( 3, 4, 1 );
    matrix.setPixel( 10, 2, 1 );
    matrix.setPixel( 9, 3, 1 );
    matrix.setPixel( 10, 3, 1 );
    matrix.setPixel( 11, 3, 1 );
    matrix.setPixel( 10, 4, 1 );
  }
}

void playDeadSound() {
  buzzer.setVolume(255);
  for (uint8_t i = 0; i < 8; i++) {
    buzzer.setFrequency(200);
    delay(45);
    buzzer.setFrequency(150);
    delay(45);
  }
  buzzer.setVolume(0);
}

void playAliveSound() {
  buzzer.setVolume(255);
  buzzer.setFrequency(600);
  delay(120);
  buzzer.setFrequency(500);
  delay(120);
  buzzer.setVolume(0);
}


void playExplodingSound() {
  uint8_t soundDelay = 45;
  delay(400);
  buzzer.setVolume(255);
  for (uint8_t i = 0; i < 2; i++) {
    buzzer.setFrequency(550); 
    delay(soundDelay);
    buzzer.setFrequency(404); 
    delay(soundDelay);
    buzzer.setFrequency(315); 
    delay(soundDelay);
    buzzer.setFrequency(494); 
    delay(soundDelay);
    buzzer.setFrequency(182); 
    delay(soundDelay);
    buzzer.setFrequency(260); 
    delay(soundDelay);
    buzzer.setFrequency(455); 
    delay(soundDelay);
    buzzer.setFrequency(387); 
    delay(soundDelay);
    buzzer.setFrequency(340); 
    delay(soundDelay);
    buzzer.setFrequency(550); 
    delay(soundDelay);
    buzzer.setFrequency(404); 
    delay(soundDelay);
    buzzer.setFrequency(315); 
    delay(soundDelay);
    buzzer.setFrequency(494); 
    delay(soundDelay);
    buzzer.setFrequency(182); 
    delay(soundDelay);
    buzzer.setFrequency(260); 
    delay(soundDelay);
    buzzer.setFrequency(455); 
    delay(soundDelay);
    buzzer.setFrequency(387); 
    delay(soundDelay);
    buzzer.setFrequency(340); 
    delay(soundDelay);
  }
  buzzer.setVolume(0);
}

void playBombingSound() {
  buzzer.setVolume(255);
  for (int i = 1000; i >= 200; i = i- 50) {
    buzzer.setFrequency(i);
    delay(45);
  }
  buzzer.setVolume(0);  
}

void setBeacon() {
  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor(teamId);
  oBeacon.setMinor(state);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04
  
  std::string strServiceData = "";
  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData(); 
  oAdvertisementData.addData(strServiceData);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  pAdvertising->start();
}
