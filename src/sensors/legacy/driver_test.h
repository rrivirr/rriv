/* 
 *  RRIV - Open Source Environmental Data Logging Platform
 *  Copyright (C) 20202  Zaven Arra  zaven.arra@gmail.com
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

// #ifndef WATERBEAR_DRIVER_TEST
// #define WATERBEAR_DRIVER_TEST



// #include "sensors/sensor.h"

// //split into drivers
// typedef struct fig_e13_methane_type{
//     // Figaro NGM2611-E13 Methane Sensor Module
//     common_config_sensor common;

//     char padding[32];
// }fig_methane_sensor;

// typedef struct as_mini_conductivity_k1_type{
//     //Atlas Scientific Mini Conductivity Probe K 1.0
//     common_config_sensor common;

//     char padding[32];
// }as_conductivity_sensor;

// typedef struct as_rgb_type{
//     //Atlas Scientific EZO-RGB Embedded Color Sensor
//     common_config_sensor common;

//     char padding[32];
// }as_rbg_sensor;

// typedef struct as_co2_type{
//     //Atlas Scientific EZO-CO2 Embedded NDIR Carbon Dioxide Sensor
//     common_config_sensor common;

//     char padding[32];
// }as_co2_sensor;

// typedef struct af_gps_type{
//     //Adafruit GPS - model?
//     common_config_sensor common;

//     char padding[32];
// }gps_sensor;

// // void * for reading struct and sensor type based on first 2 bytes of structs

// void init_struct(char sensor_slot); //populate struct with defaults based on sensor type
// void new_config(short sensor_slot); // malloc an empty config struct based on sensor type?
// void get_sensor_type(char sensor_slot); // return sensor type from eeprom


// #endif