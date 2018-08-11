#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

String towerName = "toren01";
uint8_t leadingTeam;



#define SERVICE_UUID        "111fc111-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "11154111-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  Serial.begin(9600);
  Serial.println("===== starting tower... =====");
  Serial.print("Tower name: ");
  Serial.println(towerName);

}

void loop() {
  delay(500);
}
