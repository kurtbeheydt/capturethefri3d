#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <sstream>
#include <string>

const String towerName = "toren01";
uint8_t leadingTeam;

#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"

BLEScanResults foundDevices;

int scanTime = 10; // Seconds

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

void setup() {
  Serial.begin(9600);
  Serial.println("===== starting tower... =====");
  Serial.print("Tower name: ");
  Serial.println(towerName);

  BLEDevice::init(towerName.c_str());
}

void loop() {
  Serial.println("===== Scanning... =====");
  BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  foundDevices = pBLEScan->start(scanTime);
  
  Serial.println("===== Number of badges: ===== ");
  int counter=0;
  for (int i=0; i<foundDevices.getCount(); i++) {
    int bleRssi = foundDevices.getDevice(i).getRSSI();
    Serial.print("Device rssi: ");
    Serial.println(bleRssi);
    /*
    uint8_t *payload = foundDevices.getDevice(i).getPayload();
    Serial.print("Device payload: ");
    Serial.println(String((uint32_t)payload));
    */ 
    // get manufacturerData
    String bleManufacturerData = getManufacturerData(i);
    Serial.print("Device manufacturerdata: ");
    Serial.println(bleManufacturerData);

// 4c0002154d6fc88bbe756698da486866a36ec78e0001000300 team 1 bombing
// 4c0002154d6fc88bbe756698da486866a36ec78e0001000100 team 1 conquering
// 4c0002154d6fc88bbe756698da486866a36ec78e0002000300 team 2 bombing
// 4c0002154d6fc88bbe756698da486866a36ec78e0002000100 team 2 conquering
     
  }
  Serial.println(counter);
}


String getManufacturerData(int deviceId) {
  std::stringstream ss;
  char *pHex = BLEUtils::buildHexData(nullptr, (uint8_t*)foundDevices.getDevice(deviceId).getManufacturerData().data(), foundDevices.getDevice(deviceId).getManufacturerData().length());
  ss << pHex;
  free(pHex);
  std::string data = ss.str();
  return data.c_str();
}

