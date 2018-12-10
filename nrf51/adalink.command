python -m adalink.main -v nrf51822 -p stlink --program-hex ../../waterteam-arduino/nrf51/blespifriend_s110_xxac_0_8_0_170925_blespifriend.hex --program-hex ../../waterteam-arduino/nrf51/blespifriend_s110_xxac_0_8_0_170925_blespifriend_signature.hex

python -m adalink.main -v nrf51822 -p stlink --info

python -m adalink.main -v nrf51822 -p stlink --program-hex "../Adafruit_nRF51822_Flasher/Adafruit_BluefruitLE_Firmware/softdevice/s110_nrf51_8.0.0_softdevice.hex" --program-hex "../Adafruit_nRF51822_Flasher/Adafruit_BluefruitLE_Firmware/bootloader/bootloader_0002.hex" --program-hex "../Adafruit_nRF51822_Flasher/Adafruit_BluefruitLE_Firmware/0.7.0/blespifriend/blespifriend_s110_xxac_0_7_0_160628_blespifriend.hex" --program-hex "../Adafruit_nRF51822_Flasher/Adafruit_BluefruitLE_Firmware/0.7.0/blespifriend/blespifriend_s110_xxac_0_7_0_160628_blespifriend_signature.hex"
