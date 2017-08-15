/**
 * Radar
 *
 * Scan for wifi devices.
 *
 */

// Expose Espressif SDK functionality
extern "C" {
    #include "user_interface.h"
}

#include <Arduino.h>
#include "sniffer/sniffer.hpp"

void setup()
{
    // Initialize serial communication
    Serial.begin(9600);
    delay(5);
    Serial.println();
    Serial.println("Initializing system.");

    wifi_set_opmode(STATION_MODE); // Promiscuous mode only works with station mode
    wifi_station_disconnect(); // API requirement before enable promiscuous mode.
    wifi_set_channel(1);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(sniffer_callback);
    wifi_promiscuous_enable(1);

    Serial.println("System initialized.");

}

void loop()
{
    delay(10);
}
