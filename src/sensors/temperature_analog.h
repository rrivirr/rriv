// #ifndef WATERBEAR_TEMPERATURE_ANALOG
// #define WATERBEAR_TEMPERATURE_ANALOG

// #include "sensor.h"

// typedef struct temperature_analog_type
// {
//     // 10k ohm NTC 3950b thermistor
//     common_config_sensor common;
//     byte calibrated; // 1 byte => bit mask
//     byte sensor_port; // 1 byte => add into bit mask (4bits)
//     unsigned short m; // 2bytes, slope
//     int b; // 4bytes, y-intercept
//     unsigned int cal_timestamp; // 4byte epoch timestamp at calibration
//     short int c1; // 2bytes for 2pt calibration
//     short int v1; // 2bytes for 2pt calibration
//     short int c2; // 2bytes for 2pt calibration
//     short int v2; // 2bytes for 2pt calibration

//     char padding[12];
// }temperature_analog_sensor;

// void testWriteConfig(short sensor_slot);
// void testReadConfig(short sensor_slot, temperature_analog_sensor *destination);
// void getDefaultsTAS(temperature_analog_sensor *fillValues);
// void printSensorConfig(temperature_analog_sensor toPrint);

// #endif
