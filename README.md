# Capture the Fri3d
A fri3d badge adaptation of a fps tower defence game where players are teaming up to conquer and defend towers.

Code created for the 2018 [fri3d camp badge](https://github.com/Fri3dCamp/badge)

Libraries needed:
- [ESP32 board](https://github.com/espressif/arduino-esp32) 
- [fri3dBadge](https://github.com/Fri3dCamp/Fri3dBadge) 
- [ESP32 BLE Arduino](https://github.com/nkolban/ESP32_BLE_Arduino) (use them to replace the default ESP32 BLE library) 

## gameplay and rules

_coming ..._

## player badges

Flash the badges of the players with the player code.
Change the teamname in your code before flashing to different teams.

## tower badges

The sketch is too big for the default partition on the ESP-WROOM-32 chip. 
Before you can upload your code, you need to manually increase this by editing two files.

In `~Arduino~/hardware/espressif/esp32/tools/partitions/default.csv`
find this part:

```
app0,     app,  ota_0,   0x10000, 0x140000,
app1,     app,  ota_1,   0x150000,0x140000,
```

replace it with:

```
app0,     app,  ota_0,   0x10000, 0x240000,
app1,     app,  ota_1,   0x250000,0x40000,
```

In `~Arduino~/hardware/espressif/esp32/boards.txt` 
find this part:

```
esp32.name=ESP32 Dev Module

esp32.upload.tool=esptool
esp32.upload.maximum_size=1310720
```

replace the maximum_size with this

```
esp32.upload.maximum_size=2310720
```

## backend

basic php-script.
Upload all files to a php-enabled webserver.
Make sure data.json is writable (chmod 777).

## dummy player

Just for testing purposes, a lightweight variant to create dummy conquerers for ESP-WROOM-32 dev boards.