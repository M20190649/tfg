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

#include <ESP8266WiFi.h>

/**
 * Callback for promiscuous mode
 */
static void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length)
{
    
}

void setup()
{
    // Initialize serial communication
    Serial.begin(9600);
    Serial.println();
    Serial.print("Initializing system.");

    delay(10);
    wifi_set_opmode(STATION_MODE); // Promiscuous mode only works with station mode
    wifi_station_disconnect(); // API requirement before enable promiscuous mode.
    wifi_set_channel(1);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(sniffer_callback);
    wifi_promiscuous_enable(1);

    Serial.print("System initialized.");

}

void loop()
{

}