# waterteam-embedded

Install the file COLUMNS.TXT into the root directory of your SD Card, and change field names 3-8 to reflect the sensors attached to analog inputs 0-5 (A0 - A5).

Calibration of conductivity probe via Serial:
For Serial: connect programming header to nucleo board: Tx->RX, RX->Tx, SWD GND->GND
Use a serial monitor @ baud rate 115200
You may need to restart the nucleo board by hitting the Black reset button
If the output gets stuck at 'scanning...' then you will need to power cycle the waterbear shield, this can be done by unplugging the power back then plugging it back in.
You should see consistent output after initial setup steps (if reset/first start)
Type or copy&paste entire command including ><:
>WT_CONFIG<
Which will queue to enter config mode. The serial output should immediately change upon entering in the command.
Next we'll be giving commands for a two-point calibration using the recommended Atlas Scientific K1.0 calibration solutions which are Low:12880uS, and High:80000uS, if you are using custom calibration points, change the numerical value in the Low and High commands accordingly.
1. Have the probe in open air and let it stabilize at 0
>CAL_DRY<
Any output should change to 0 at this point
2. Place probe in low calibration solution, and wait for readings to stabilize. You may want to shake the probe a little to dislodge any air bubbles that may be in the sensing area.
>CAL_LOW:12880<
The readings will not change at this point.
3. Clean probe (I rinse in water and dry to prevent contamination of calibration fluids), then place into High calibration solution, again shaking it to dislodge air bubbles, and waiting for readings to stabilize.
>CAL_HIGH:80000<
At this point you should see the results change to ~80000
4. It would be wise to test the probe at various dilutions to confirm that it is working correctly. If anything seems awry, start over from step 1, as new data will overwrite the old.
5. Reset to reenter normal operations. [Though currently the software may need to be altered for that]

To configure time:
Not necessary but can enter >WT_CONFIG< mode
Serial command is:
>WT_SET_RTC:[insert epoch time]<
example: >WT_SET_RTC:1605578020<

Check version of Maple is at least: framework-arduinoststm32-maple 2.10000.200103 (1.0.0)
  This impacts some commands in the platform.ini [build flag, board build]
