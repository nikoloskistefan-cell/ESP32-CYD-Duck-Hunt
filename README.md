# Duck Hunt – ESP32 CYD Touchscreen Game

A retro duck-hunting game for the ESP32 Cheap Yellow Display (CYD). Tap the touchscreen to shoot flying ducks, manage your ammunition, and help the dog retrieve your hits.

## Links

- 🎥 Video: [Duck Hunt on YouTube Shorts](https://youtube.com/shorts/ZBqUziBsLYI)
- 🛒 ESP32 CYD display used in this project: [Temu item link](https://temu.to/k/ea65oy4oun8)

The Temu link may be an affiliate link; it does not change the price for you.

## Features

- 320×240 color graphics on an ILI9341 TFT
- XPT2046 resistive touchscreen controls
- Duck flight animation with wing frames
- Buzzer sound effects and title music
- Rounds, ammunition, score, and persistent high score

## Hardware

- ESP32-2432S028 / ESP32 CYD
- ILI9341 320×240 TFT
- XPT2046 touchscreen
- On-board buzzer

The sketch uses the CYD pin assignments defined in `duck-hunt.ino`.

## Arduino setup

1. Install the ESP32 board package and select an ESP32 Dev Module (or the matching CYD board).
2. Install the libraries listed below through the Arduino Library Manager.
3. Open `duck-hunt.ino` and upload it to the board.

Recommended libraries:

- Adafruit GFX Library
- Adafruit ILI9341
- XPT2046_Touchscreen

`SPI` and `Preferences` are provided by the ESP32 Arduino core.

## License

The game sketch is released under the MIT License. See [LICENSE](LICENSE).

## Notes

This repository contains the game sketch only. No Wi-Fi credentials, API keys, Odoo credentials, or other private configuration are required.
