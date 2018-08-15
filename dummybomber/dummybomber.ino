#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>
#include <sstream>
#include <string>

const String teamTag = "R"; // choose from R|G|B
uint16_t teamId;
uint64_t playerId;
String playerTag;

#define SERVICE_UUID        "111fc111-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "11154111-36e1-4688-b7f5-ea07361b26a8"
#define BEACON_UUID         "8ec76ea3-6668-48da-9866-75be8bc86f4d"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;

enum State { STATE_CONQUER, STATE_CONQUERING, STATE_BOMB, STATE_BOMBING, STATE_UNDERBOMBING, 
             STATE_EXPLODING, STATE_FIGHT, STATE_FIGHTING, STATE_DYING, STATE_DEAD };
State state = STATE_BOMB;

// fightmode vars
long fight_startTime = 0; // millis to capture start of fightmode
const long fight_maxDurationTime = 5; // time in seconds
long fight_remainingTime;
float fight_startAccel[3];
float fight_maxDeviation = 2.00;

// bombingmode vars
long bomb_startTime = 0; // millis to capture start of bombmode
const uint8_t bomb_durationTime = 11;
const uint8_t bomb_explodingTime = 5;
long bomb_remainingTime;

// conquerer vars
long conq_lastScanTime = 0; // millis to capture start of conquerermode
const uint8_t conq_scanBombersDuration = 1; // TODO make larger
const uint8_t conq_scanBombersIntervalTime = 1;

// ui
// I know there is a button class, but this gave errors in multi_heap.c, 
// see https://github.com/Fri3dCamp/Fri3dBadge/issues/4
#define BUTTON0_PIN 36
#define BUTTON1_PIN 39
int buttons[2] = {BUTTON0_PIN, BUTTON1_PIN};
int buttonPressed[2] = {0, 0};

String getManufacturerData(BLEAdvertisedDevice advertisedDevice) {
  std::stringstream ss;
  char *pHex = BLEUtils::buildHexData(nullptr, 
                                      (uint8_t*)advertisedDevice.getManufacturerData().data(), 
                                      advertisedDevice.getManufacturerData().length());
  ss << pHex;
  free(pHex);
  std::string data = ss.str();
  return data.c_str();
}

class bombingCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String bleManufacturerData = getManufacturerData(advertisedDevice);
    if (bleManufacturerData.substring(0, 40) == "4c0002154d6fc88bbe756698da486866a36ec78e") { // iBeacon prefix
      int teamId = bleManufacturerData.substring(43, 44).toInt(); // get major
      int state = bleManufacturerData.substring(47, 48).toInt(); // get minor
      int bleRssi = advertisedDevice.getRSSI() * -1; // the lower, the better
      Serial.print("Found a badge for team ");
      Serial.print(teamId, DEC);
      Serial.print(" and state ");
      Serial.print(state, DEC);
      Serial.print(" and rssi ");
      Serial.println(bleRssi, DEC);   
      if (state == STATE_BOMBING) { // in bombing mode
        state = STATE_UNDERBOMBING;
      } else if (state == STATE_EXPLODING) { // in exploding mode
        state = STATE_EXPLODING;
      }
    }
  }
};

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
}

void loop() {
  switch (state) {
    case STATE_BOMB:
      startBombing();
      break;
    case STATE_BOMBING:
      if (millis() > (bomb_startTime + 1000)) {
        bomb_startTime = millis();
        bomb_remainingTime = bomb_remainingTime - 1000;
      }
      if (bomb_remainingTime <= 1) {
        startExploding();
      }
      break;
    case STATE_EXPLODING:
      state = STATE_DYING;
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
  }
}

void startExploding() {
  if (state == STATE_BOMBING) {
    Serial.println("starting exploding ...");
    state = STATE_EXPLODING; 
    
    // change ble mode
    pAdvertising->stop();
    setBeacon();
 
    delay((bomb_explodingTime - 1) * 1000); // keep sending the exploding signal to 
                                            // other badges who are under bombing
  }
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
