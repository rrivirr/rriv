/*
void setupEZOSerial(){

  Serial1.begin(ezoBaud);

  Serial2.println("Start EZO setup");

  inputstring.reserve(20);                            //set aside some bytes for receiving data from the PC

  ezo_ec.send_cmd_no_resp("*ok,0");             //send the command to turn off the *ok response

  // in order to use multiple circuits more effectively we need to turn off continuous mode and the *ok response
  Serial2.print("C,? : ");
  ezo_ec.send_cmd("c,?", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);                  //print the modules response

  Serial2.print("K,? : ");
  ezo_ec.send_cmd("K,?", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);


  ezo_ec.send_cmd("O,EC,1", response_data, bufferlen); // send it to the module of the port we opened
  ezo_ec.send_cmd("O,TDS,0", response_data, bufferlen); // send it to the module of the port we opened
  ezo_ec.send_cmd("O,S,0", response_data, bufferlen); // send it to the module of the port we opened

  Serial2.print("O,? : ");
  ezo_ec.send_cmd("O,?", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);

  ezo_ec.send_cmd_no_resp("c,0");               //send the command to turn off continuous mode
                                          //in this case we arent concerned about waiting for the reply
  ezo_ec.flush_rx_buffer();                     //clear all the characters that we received from the responses of the above commands

  Serial2.println("Done with EZO setup");
}
*/

/*
void stopEZOSerial(){

  ezo_ec.send_cmd("Sleep", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);
  Serial2.flush();

  Serial1.end();

}
*/


    // EZO
    // wake/sleep.  Or re-run setup
    /*
    Serial2.print(ezo_ec.get_name());     //print the modules name
    Serial2.print(": ");
    Serial2.println(response_data);                  //print the modules response
    response_data[0] = 0;                           //clear the modules response

    ezo_ec.send_read();
    Serial2.print("EZO Reading:");
    float ecValue = ezo_ec.get_reading();
    Serial2.print(ecValue);
    Serial2.println();
    sprintf(values[4], "%4f", ecValue); // stuff EC value into values[4] for the moment.
    */


    //ezo_ec = new Ezo_board(&WireTwo, 0x64);

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
