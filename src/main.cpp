/**
 * Radar
 *
 * Scan for wifi devices.
 *
 */

// Expose Espressif SDK functionality
extern "C" {
    #include "user_interface.h"
    #include "sntp.h"
}

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <sniffer.hpp>
#include <Time.h>


#define CHANNEL_HOP_INTERVAL_MS   1000 // We will change WiFi channel every second
static os_timer_t channel_hop_timer; // Software timer for channel surgfing

void setup()
{
    // Initialize serial communication
    Serial.begin(9600);
    delay(5);
    Serial.println();
    Serial.println("Initializing system.");

    // WiFi connection
    WiFi.begin("SSID", "PASSWORD");
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    //Get time through SNTP
    Serial.print("Getting timestamp from server.");
    sntp_setservername(0, (char*)"hora.roa.es");
    sntp_setservername(1, (char*)"pool.ntp.org");
    sntp_setservername(2, (char*)"time.nist.gov");
    sntp_set_timezone(+2);
    sntp_init();

    uint32_t current_stamp = 0;
    while (current_stamp == 0) {
        current_stamp = sntp_get_current_timestamp();
        delay(100);
        Serial.print(".");
    }
    setTime(current_stamp);
    sntp_stop();

    Serial.printf("\nSNTP OK\nTimestamp:	%d,	%s	\n",current_stamp, sntp_get_real_time(current_stamp));    

    Serial.println("Starting sniffer mode.");
    // Promiscuous mode
    wifi_set_opmode(STATION_MODE); // Promiscuous mode only works with station mode
    wifi_station_disconnect(); // API requirement before enable promiscuous mode.
    wifi_set_channel(1);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(Sniffer::sniffer_callback);
    wifi_promiscuous_enable(1);

    // Set callback function for channel surfing timer and enable timer.
    os_timer_setfn(&channel_hop_timer, (os_timer_func_t *) Sniffer::channel_hop, NULL);
    os_timer_arm(&channel_hop_timer, CHANNEL_HOP_INTERVAL_MS, true);
    
    Serial.println("System initialized.");

}

void loop()
{

}

