#include "temperature_analog.h"
#include "system/eeprom.h"
#include "system/monitor.h"

void testWriteConfig(short sensor_slot)
{
    Monitor::instance()->writeDebugMessage(F("testWriteConfig"));
    Serial2.flush();
    temperature_analog_sensor testValues;
    getDefaultsCommon(&testValues.common);
    getDefaultsTAS(&testValues);
    
    char buffer[200];
    sprintf(buffer, "testValues:\nCCS: %d, %d, %d, %d, %s, %s, %s\nTAS: %d, %d, %d, %d, %d, %d, %d, %d, %d, %s\n", 
        testValues.common.sensor_type, testValues.common.slot, testValues.common.burst_size,
        testValues.common.warmup, testValues.common.tag, testValues.common.tag,
        testValues.common.padding,
        testValues.calibrated, testValues.sensor_port, testValues.m, testValues.b,
        testValues.cal_timestamp, testValues.c1, testValues.v1, testValues.c2, testValues.v2,
        testValues.padding);
    Monitor::instance()->writeDebugMessage(F(buffer));
    
    writeEEPROMBytesMem(sensor_slot, &testValues, sizeof(testValues));
    Monitor::instance()->writeDebugMessage(F("finishWriteConfig"));
    Serial2.flush();
}

void getDefaultsTAS(temperature_analog_sensor *fillValues)
{
    Monitor::instance()->writeDebugMessage(F("getDefaultsTAS"));
    Serial2.flush();
    fillValues->calibrated = 1;
    fillValues->sensor_port = 1;
    fillValues->m = 12;
    fillValues->b = 1337;
    fillValues->cal_timestamp = 639587682; // 04-08-90 3:14:42 PM
    fillValues->c1 = 660;
    fillValues->v1 = 900;
    fillValues->c2 = 2300;
    fillValues->v2 = 3000;
    strcpy(fillValues->padding, "DefaultTest");
}

void testReadConfig(short sensor_slot, temperature_analog_sensor *destination)
{
    Monitor::instance()->writeDebugMessage(F("testReadConfig"));
    Serial2.flush();

    readEEPROMBytesMem(sensor_slot, (void *) destination, sizeof(*destination));
    Monitor::instance()->writeDebugMessage(F("finishReadConfig"));
    Serial2.flush();
}

void printSensorConfig(temperature_analog_sensor toPrint)
{
    Monitor::instance()->writeDebugMessage(F("printSensorConfig"));
    Serial2.flush();
    char buffer[200];

    sprintf(buffer, "CCS: %d, %d, %d, %d, %s, %s, %s\nTAS: %d, %d, %d, %d, %d, %d, %d, %d, %d, %s",
        toPrint.common.sensor_type, toPrint.common.slot, toPrint.common.burst_size,
        toPrint.common.warmup, toPrint.common.tag, toPrint.common.tag, toPrint.common.padding,
        toPrint.calibrated, toPrint.sensor_port, toPrint.m, toPrint.b,
        toPrint.cal_timestamp, toPrint.c1, toPrint.v1, toPrint.c2, toPrint.v2, toPrint.padding);
    Monitor::instance()->writeDebugMessage(F(buffer));
}