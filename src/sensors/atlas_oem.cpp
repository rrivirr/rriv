#include "atlas-oem.h"

EC_OEM * oem_ec;

void setupEC_OEM(TwoWire * wire) {

    Serial2.println("EC I2C setup");
    Serial2.println("Setting up board");

    oem_ec = new EC_OEM(wire, NONE_INT, ec_i2c_id);
    bool awoke = oem_ec->wakeUp();
    Serial.println("Device addr EC: "+String(oem_ec->getStoredAddr()) );
    Serial.println("Device type EC: "+String(oem_ec->getDeviceType()) );
    Serial.println("Firmware EC: "+String(oem_ec->getFirmwareVersion()) );
    Serial.println("Awoke: "+String(awoke));
    Serial.println("Hibernating: "+String(oem_ec->isHibernate()) );

    oem_ec->singleReading();
    struct param_OEM_EC parameter;
    parameter = oem_ec->getAllParam();
    Serial.println("salinity= " + String(parameter.salinity)+
                   "\nconductivity= " +String(parameter.conductivity)+
                   "\ntds= " +String(parameter.tds)+
                   "\nSalinity stable = "+(oem_ec->isSalinityStable()?"yes":"no")
                   );


    //ezo_ec = new Ezo_board(&Wire2, 0x64);

    // inputstring.reserve(20);

    // Serial2.println("Turning light on");
    // ezo_ec->send_cmd("L,1");
    // delay(1000);
    // Serial2.println("Turning light off");
    // ezo_ec->send_cmd("L,0");
    // delay(1000);
    // Serial2.println("Turning light on");
    // ezo_ec->send_cmd("L,1");
    // delay(1000);

    // Set probe type
    // ezo_ec->send_cmd("K,1.0");
    // delay(300);
    //
    // // Set outputs
    // ezo_ec->send_cmd("O,EC,1");
    // delay(300);
    // ezo_ec->send_cmd("O,TDS,0");
    // delay(300);
    // ezo_ec->send_cmd("O,S,0");
    // delay(300);


    oem_ec->setLedOn(true);
    //oem_ec->setLedOn(false);
    //oem_ec->setLedOn(true);
    oem_ec->setProbeType(1.0);

    Serial2.println("Done with EZO I2C setup");

}


void hibernateEC_OEM(){
    oem_ec->setHibernate();
}

void clearECCalibrationData(){
    oem_ec->clearCalibrationData();
}

void setECDryPointCalibration(){
    oem_ec->setCalibration(DRY_CALIBRATION);
}

void setECLowPointCalibration(float lowPoint){
    oem_ec->setCalibration(LOW_POINT_CALIBRATION, lowPoint);
}

void setECHighPointCalibration(float highPoint){
    oem_ec->setCalibration(HIGH_POINT_CALIBRATION, highPoint);
}

bool readECDataIfAvailable(float * ecValue){
    bool newDataAvailable = oem_ec->singleReading();

    if(newDataAvailable){
      *ecValue = oem_ec->getConductivity();
      oem_ec->clearNewDataRegister();
    }
}