#include "Arduino.h"
#include "Adafruit_BluefruitLE_UART.h"


class WaterBear_Control
{
  private:
    /*Stream *mySerial;
    HardwareSerial * hs;
    Adafruit_BluefruitLE_UART * ble;
    */


  public:

    static int state;

    static void processControlCommands(Stream * mySerial);
    static void processControlCommands(HardwareSerial &port);
    static void processControlCommands(Adafruit_BluefruitLE_UART &ble);

    static bool ready(HardwareSerial &port);
    static bool ready(Adafruit_BluefruitLE_UART &ble);

#if SOFTWARE_SERIAL_AVAILABLE
    static void processControlCommands(SoftwareSerial &port);
#endif
};
