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
#include <sniffer.hpp>

#define CHANNEL_HOP_INTERVAL_MS   1000 // We will change WiFi channel every second
static os_timer_t channel_hop_timer; // Software timer for channel surgfing

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

    // Set callback function for channel surfing timer and enable timer.
    os_timer_setfn(&channel_hop_timer, (os_timer_func_t *) channel_hop, NULL);
    os_timer_arm(&channel_hop_timer, CHANNEL_HOP_INTERVAL_MS, true);
    
    Serial.println("System initialized.");

}

void loop()
{

}

