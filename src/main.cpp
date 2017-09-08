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
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <sniffer.hpp>
#include <Time.h>

uint32_t uptime;//debug

#define CHANNEL_HOP_INTERVAL_MS   1000 // We will change WiFi channel every second
#define SEND_DETECTIONS_INTERVAL_MS 20000 // Send detections to server every minute
static os_timer_t channel_hop_timer; // Software timer for channel surgfing
static os_timer_t send_detections_timer; // Software timer for send detections to server
bool channel_hop_flag = false;
bool send_detections_flag = false;

// It`s best not to do long jobs inside isr (timer callback)
// , so only flag the timers and check flags in main loop.

void channel_hop_isr () {
    channel_hop_flag = true;
}

void send_detections_isr () {
    send_detections_flag = true;
}

void connect_wifi() {
    WiFi.begin("HOME", "DDLpkP1u.");
    delay(500);
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
}

void send_detections() {
    yield();
    wifi_promiscuous_enable(0);
    delay(50);
    connect_wifi();

    // Memory pool for JSON object tree (bytes).
    // StaticJsonBuffer allocates memory on the stack, it can be
    // replaced by DynamicJsonBuffer which allocates in the heap.
    StaticJsonBuffer<3000> json_buffer;

    // Create the root of the object tree.
    //
    // It's a reference to the JsonObject, the actual bytes are inside the
    // JsonBuffer with all the other nodes of the object tree.
    // Memory is freed when jsonBuffer goes out of scope.
    JsonObject& root = json_buffer.createObject();
    root["node"] = WiFi.macAddress();
    JsonArray& devices = root.createNestedArray("devices");
    JsonArray& timestamps = root.createNestedArray("timestamps");

    unsigned long start = millis();
    for (std::map<String,uint32_t>::iterator it=Sniffer::sta_detected.begin(); it!=Sniffer::sta_detected.end(); ++it){
        devices.add(it->first);
        timestamps.add(it->second);
        yield();
    }
    unsigned long end = millis();
    Serial.printf("\nMillis: %d\n", (end-start));
    yield();
    Serial.println();
    root.printTo(Serial);
    Serial.printf("\nUPTIME:	%s	\n",sntp_get_real_time(uptime));//debug
    yield();
    wifi_station_disconnect(); // API requirement before enable promiscuous mode.
    wifi_set_opmode(STATION_MODE); // Promiscuous mode only works with station mode
    wifi_promiscuous_enable(1);
}

void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    delay(5);
    Serial.println();
    Serial.println("Initializing system.");

    // WiFi connection
    connect_wifi();

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
        delay(500);
        Serial.print(".");
    }
    setTime(current_stamp);
    sntp_stop();
    uptime = current_stamp;// debug

    Serial.printf("\nSNTP OK\nTimestamp:	%d,	%s	\n",current_stamp, sntp_get_real_time(current_stamp));

    Serial.println("Starting sniffer mode.");
    // Promiscuous mode

    wifi_station_disconnect(); // API requirement before enable promiscuous mode.
    wifi_set_opmode(STATION_MODE); // Promiscuous mode only works with station mode
    wifi_set_channel(1);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(Sniffer::sniffer_callback);
    wifi_promiscuous_enable(1);

    // Set callback function for channel surfing timer and enable timer.
    os_timer_setfn(&channel_hop_timer, (os_timer_func_t *) channel_hop_isr, NULL);
    os_timer_arm(&channel_hop_timer, CHANNEL_HOP_INTERVAL_MS, true);

    // Set callback function for send detections to server timer and enable timer.
    os_timer_setfn(&send_detections_timer, (os_timer_func_t *) send_detections_isr, NULL);
    os_timer_arm(&send_detections_timer, SEND_DETECTIONS_INTERVAL_MS, true);

    Serial.println("System initialized.");

}

void loop()
{
    if (channel_hop_flag) {
        Sniffer::channel_hop();
        channel_hop_flag = false;
    }

    if (send_detections_flag) {
        send_detections();
        send_detections_flag = false;
    }
}
