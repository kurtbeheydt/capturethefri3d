#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>

const String teamTag = "B"; // choose from R|G|B
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
             STATE_FIGHT, STATE_FIGHTING, STATE_DYING, STATE_DEAD };
State state = STATE_CONQUERING;

void setup() {
  Serial.begin(9600);

  // player details
  setTeamId();
  Serial.println("===== starting dummyplayer... =====");
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
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue(playerTag.c_str());
  pService->start();  
  pAdvertising = pServer->getAdvertising();

  setBeacon();
}

void loop() {
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
