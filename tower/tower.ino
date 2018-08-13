#include <BLEDevice.h>
#include <sstream>
#include <string>
#include <WiFi.h>
#include <HTTPClient.h>

#include "config.h" // wifi and backend settings

const String towerName = "toren01";
uint8_t currentLeadingTeam;
const uint8_t teamCount = 3;

#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"

BLEScanResults foundDevices;
uint8_t scanData[4][2] = {{0,0},{0,0},{0,0}};
uint8_t winningTeamData[3] = {0,0,0};

int scanTime = 5; // Seconds

void resetScanData() {
  for (int i = 1; i <= teamCount; i++) {
    scanData[i][0] = 0;
    scanData[i][1] = 0;
  }
}

void calculateWinningTeam() {
  winningTeamData[0] = 0;
  winningTeamData[1] = 0;
  winningTeamData[2] = 0;
  for (int i = 1; i <= teamCount; i++) {
    if (scanData[i][0] > winningTeamData[1]) { // more players at the token -> winner
      winningTeamData[0] = i;
      winningTeamData[1] = scanData[i][0];
      winningTeamData[2] = scanData[i][1];
    } else if (scanData[i][0] == winningTeamData[1]) { // equal number of players
      if (scanData[i][1] < winningTeamData[2]) { // lower total rssi -> winner
        winningTeamData[0] = i;
        winningTeamData[1] = scanData[i][0];
        winningTeamData[2] = scanData[i][1];       
      }
    }
  }

  if (winningTeamData[0] > 0) {
    currentLeadingTeam = winningTeamData[0];
  }
}

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

String getTeamId(uint8_t teamid) {
  if (teamid == 1) {
    return "rood";
  } else if (teamid == 2) {
    return "groen";
  } else if (teamid == 3) {
    return "blauw";
  }
  return "niemand";
}

void postScore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;   

    String gameData = "{\"secret\": \"" + secret + "\"," +
                        "\"name\": \"" + towerName + "\"," +  
                        "\"currentLeadingTeam\": \"" + getTeamId(currentLeadingTeam) + "\"," + 
                        "\"teamData\": {" + 
                          "\"team1\": {" +
                            "\"teamId\": \"rood\"," + 
                            "\"teamCount\": " + String((uint16_t)scanData[1][0]) +  
                          "}," +
                          "\"team2\": {" +
                            "\"teamId\": \"groen\"," + 
                            "\"teamCount\": " + String((uint16_t)scanData[2][0]) +  
                          "}," +
                          "\"team3\": {" +
                            "\"teamId\": \"blauw\"," + 
                            "\"teamCount\": " + String((uint16_t)scanData[3][0]) +  
                          "}" +
                        "}" +
                      "}";
    
    http.begin(backendUrl + "?data=" + gameData);
    http.addHeader("Content-Type", "text/plain");
    
    int httpResponseCode = http.GET(); // POST request didn't want to work
    
    if (httpResponseCode > 0) {
      String response = http.getString();                       
      Serial.println(httpResponseCode);  
      Serial.println(response);         
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // 4c0002154d6fc88bbe756698da486866a36ec78e0001000300 team 1 bombing
    // 4c0002154d6fc88bbe756698da486866a36ec78e0001000100 team 1 conquering
    // 4c0002154d6fc88bbe756698da486866a36ec78e0002000300 team 2 bombing
    // 4c0002154d6fc88bbe756698da486866a36ec78e0002000100 team 2 conquering
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
      if (state == 1) { // only players in conquer mode
        scanData[teamId][0] = scanData[teamId][0] + 1; // add a teammember
        scanData[teamId][1] = scanData[teamId][1] + bleRssi; // add to total rssi
      }
    }
  }
};

void setup() {
  Serial.begin(9600);
  Serial.println("===== Starting tower... =====");
  Serial.print("Tower name: ");
  Serial.println(towerName);

  // connecting to wifi
  Serial.println("===== Connecting to wifi... =====");
  WiFi.begin(ssid, password);
  Serial.print("Establishing connection to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  } 
  Serial.println(" Connected to network");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());

  // starting ble
  Serial.println("===== Starting ble... =====");
  BLEDevice::init(towerName.c_str());
}

void loop() {
  Serial.println("===== Scanning... =====");
  // TODO flash lights while scanning
  
  resetScanData();
  BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  foundDevices = pBLEScan->start(scanTime);

  Serial.println("===== Scan Summary... =====");
  for (int i = 1; i <= teamCount; i++) {
    Serial.print("Team ");
    Serial.print(i, DEC);
    Serial.print(" has ");
    Serial.print(scanData[i][0], DEC);
    Serial.print(" player(s), with a total rssi of ");
    Serial.println(scanData[i][1], DEC);
  }
  
  Serial.println("===== Winning team... =====");
  calculateWinningTeam();
  Serial.print("Winning team: ");
  Serial.print(winningTeamData[0], DEC);
  Serial.print(" with ");
  Serial.print(winningTeamData[1], DEC);
  Serial.print(" player(s) and a total rssi of ");
  Serial.println(winningTeamData[2], DEC);

  // TODO set rgb in color of leading team
    
  Serial.println("===== Sending data to central server... =====");

  // send data to backend
  postScore();
  
  delay(5000); // TODO increase delay after testing
}

