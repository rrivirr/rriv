

# Modify the STM32 boad

Using a hacksaw, separate the programming board from the MCU board

Modify solder bridge connection on back of MCU board

* SB62 and SB63 - ON (connected)
* SB50 - OFF (disconnected)
* SB54 and SB55 - ON (connected)

Connect serial wires and test serial monitor
* Programming board TX to MCU board RX on CN9 or CN10
* Programming board RX to MCU board TX on CN9 or CN10

Test
1. Connect MCU board to WaterBear shield
1. Power MCU/WaterBear using battery
1. Connect USB from computer to programming board
1. Check for output in serial monitor, use reset button restart WaterBear
