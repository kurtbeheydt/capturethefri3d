# Capture the Fri3d
A real-life fri3d badge adaptation of a fps tower defence game where players are teaming up to conquer and defend towers.

Code created for the 2018 [fri3d camp badge](https://github.com/Fri3dCamp/badge)

Libraries needed:
- [ESP32 board](https://github.com/espressif/arduino-esp32) 
- [fri3dBadge](https://github.com/Fri3dCamp/Fri3dBadge) 
- [ESP32 BLE Arduino](https://github.com/nkolban/ESP32_BLE_Arduino) (use them to replace the default ESP32 BLE library) 

## gameplay and rules

As a team you have to conquer as many towers for as long as possible. Every tower in your team's control scores a point every 20 seconds. The towers scan for players and is controlled by the team which has the most players (with the lowest RSSI) ever 20 seconds.

- The players are divided in three teams: Red, Green and Blue. Perhaps they can visualise themselves by wearing some coloured clothining or blinking leds ;)
- there are a number of towers, also fri3d camp badges, but flashed with the tower badge sketch. The more, the better I think. They are distributed over the playing field.
- By default, a player starts dead when the switch their badge on. You can give them a live by writing an 'l' via the serial port (9600 baud). _Perhaps this will change to another procedure_
- When a player is revived, he or she starts in conquer mode. This means he or she can search for towers and when he is in bluetooth range while the tower is scanning, his badge is counted.

### fight mode

A player can challenge another player for a stand-off. If he can tag the other player, both players push button 0, keeping it pushed and release it at the same time. A countdown starts during which they can't move. If they do, the dying sound plays. The first playing having his badge play the dying sound, looses and has to switch his badge off.

### bombing mode

A player can start a bombing count-down by pressing button 1. This starts a countdown sequence in which all other players in bluetooth reach are warned that a bomb will explode. When the other players don't move quickly enough away from the bomber, they also die on explosion of the bomb.

## player badges

- Flash the badges of the players with the player code.
- Change the teamname in your code before flashing to assign a team. `const String teamTag = "R"; // choose from R|G|B`

## tower badges

- Flash the badges of the towers with this code.
- Change the towername in your code before flashing: `const String towerName = "toren01";`

Sidenotes:
- The sketch is too big for the default partition on the ESP-WROOM-32 chip. 
- Before you can upload your code, you need to manually increase this by editing two files.

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

## www

basic php-script for front-end and back-end.

- Upload all files to a php-enabled webserver.
- Make sure data.json is writable (chmod 777).

## dummy players

Just for testing purposes, ightweight variants to create dummy conquerers/bombers for ESP-WROOM-32 dev boards.