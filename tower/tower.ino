#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEBeacon.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

const char BEACON_NAME[] = "toren-1";

BLEAdvertising *pAdvertising;
BLEServer *pServer;
#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"

uint8_t currentTeam = 0;

void setBeacon() {
  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  /*
   * Major 1 voor beacons  en de minor de beacons nr
   * Major 2 en 3 en 4 etc de ploegen
   */
  oBeacon.setMajor((1 & 0xFFFF0000) >> 16); // Het is een toren
  oBeacon.setMinor(currentTeam & 0xFFFF); // het huidige team dat deze toren heeft
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
}

void initBeacon() {
  pServer = BLEDevice::createServer();
  pAdvertising = pServer->getAdvertising();
  setBeacon();
}

boolean enterBeaconMode() {  
  Serial.print("Starting advertising... ");
  pAdvertising->start();
  Serial.println("done");
  delay(2000);
  Serial.print("Stopping advertising... ");
  pAdvertising->stop();
  Serial.println("done");
}

void setup() {
  Serial.begin(115200);

  BLEDevice::init(BEACON_NAME);
  initBeacon();
}

void loop() {
  enterBeaconMode();
  delay(500);
}
