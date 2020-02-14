#include "Arduino.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BluefruitLE_SPI.h"

#define WT_CONTROL_NONE 0
#define WT_CONTROL_CONFIG 1000
#define WT_CONTROL_CAL_DRY 1001
#define WT_CONTROL_CAL_LOW 1002
#define WT_CONTROL_CAL_H 1003

//#define WT_CONTROL_CAL_HIGH 1003

class WaterBear_Control
{
  private:
    /*Stream *mySerial;
    HardwareSerial * hs;
    Adafruit_BluefruitLE_UART * ble;
    */


  public:

    static int state;

    static int processControlCommands(Stream * myStream);
    static int processControlCommands(HardwareSerial &port);
    static int processControlCommands(Adafruit_BluefruitLE_UART &ble);
    static int processControlCommands(Adafruit_BluefruitLE_SPI &ble);

    static void * getLastPayload();


    static bool ready(Stream * myStream);
    static bool ready(HardwareSerial &port);
    static bool ready(Adafruit_BluefruitLE_UART &ble);
    static bool ready(Adafruit_BluefruitLE_SPI &ble);



#if SOFTWARE_SERIAL_AVAILABLE
    static void processControlCommands(SoftwareSerial &port);
#endif
};
