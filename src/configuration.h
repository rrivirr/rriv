#ifndef WATERBEAR_CONFIGURATION
#define WATERBEAR_CONFIGURATION

#define SERIAL_BAUD 115200
#define USER_WAKE_TIMEOUT           60 * 5 // Timeout after wakeup from user interaction, seconds

// DEBUG behaviors
#define DEBUG_MEASUREMENTS false // enable log messages related to measurement & bursts
#define DEBUG_LOOP false         // don't sleep
#define DEBUG_USING_SHORT_SLEEP false // sleep for a hard coded short amount of time
#define DEBUG_TO_FILE 1   // Also send debug messages to the output file [comment out to disable]
#define DEBUG_TO_SERIAL 1 // Send debug messages to the serial interface


// Bluefruit on SPI
#define BLUEFRUIT_SPI_SCK   PB13
#define BLUEFRUIT_SPI_MISO  PB14
#define BLUEFRUIT_SPI_MOSI  PB15
#define BLUEFRUIT_SPI_CS    PB8  // Pullup
#define BLUEFRUIT_SPI_IRQ   PB9
#define BLUEFRUIT_SPI_RST   PC4




#endif