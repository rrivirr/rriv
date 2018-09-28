#include "Arduino.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BluefruitLE_SPI.h"


class WaterBear_Control
{
  private:
    /*Stream *mySerial;
    HardwareSerial * hs;
    Adafruit_BluefruitLE_UART * ble;
    */


  public:

    static int state;

    static void processControlCommands(Stream * myStream);
    static void processControlCommands(HardwareSerial &port);
    static void processControlCommands(Adafruit_BluefruitLE_UART &ble);
    static void processControlCommands(Adafruit_BluefruitLE_SPI &ble);

    static bool ready(Stream * myStream);
    static bool ready(HardwareSerial &port);
    static bool ready(Adafruit_BluefruitLE_UART &ble);
    static bool ready(Adafruit_BluefruitLE_SPI &ble);


#if SOFTWARE_SERIAL_AVAILABLE
    static void processControlCommands(SoftwareSerial &port);
#endif
};
