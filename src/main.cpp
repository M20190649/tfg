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
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <sniffer.hpp>
#include <Time.h>

#define DEBUG 0

#define CHANNEL_HOP_INTERVAL_MS   1000 // Milliseconds to change WiFi channel
#define SEND_DETECTIONS_INTERVAL_MS 20000 // Send detections to server every X milliseconds
#define SNTP_WAIT_MS 100 // Time to wait to get a response from sntp.

#define WIFI_SSID ""
#define WIFI_PASSWD ""
#define SERVER_URL ""

char station_id[32]; // ID of the esp8266, generated in setup with hashed mac (md5).

// Software timers
static os_timer_t channel_hop_timer; // Software timer for channel surfing
static os_timer_t send_detections_timer; // Software timer for send detections to server
static os_timer_t sntp_timer; // Software timer for sntp sync

bool channel_hop_flag = false;
bool send_detections_flag = false;
bool sntp_synced = false;

void channel_hop_isr () {
    channel_hop_flag = true;
}

void send_detections_isr () {
    send_detections_flag = true;
}

void ICACHE_FLASH_ATTR check_sntp_stamp_isr () {
    uint32 current_stamp;
    current_stamp = sntp_get_current_timestamp();
    if (current_stamp < 60000 ) {
        os_timer_arm(&sntp_timer, SNTP_WAIT_MS, false);
    } else {
        os_timer_disarm(&sntp_timer);
        sntp_synced = true;
    }
}

void connect_wifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
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
    DynamicJsonDocument json_doc(20000);

    json_doc["node"] = station_id;
    JsonObject detections = json_doc.createNestedObject("detections");

    for (std::map<String,uint32_t>::iterator it=Sniffer::sta_detected.begin(); it!=Sniffer::sta_detected.end(); ++it){
        detections[it->first] = it->second;
        yield();
    }

    // Clear detecions
    Sniffer::sta_detected.clear();

    #if DEBUG == 1
    serializeJsonPretty(json_doc, Serial);
    #endif
    
    yield();

    String databuf;
    serializeJson(json_doc, databuf);
    // Post detections
    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    http.POST(databuf);
    http.end();


    yield();
    wifi_station_disconnect(); // API requirement before enable promiscuous mode.
    wifi_set_opmode(STATION_MODE); // Promiscuous mode only works with station mode
    wifi_promiscuous_enable(1);
}

void ICACHE_FLASH_ATTR setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    delay(5);
    Serial.println();
    Serial.println("Initializing system.");
    Serial.println("Generating station ID.");
    // Generate station ID as MAC hashed md5
    MD5Builder _md5;
    _md5.begin();
    _md5.add(WiFi.macAddress());
    _md5.calculate();
    _md5.getChars(station_id);
    Serial.printf("Station_id: %s \n", station_id);

    yield();

    // WiFi connection
    connect_wifi();

    //Get time through SNTP
    Serial.print("Getting timestamp from server.");
    sntp_setservername(0, (char*)"hora.roa.es");
    sntp_setservername(1, (char*)"pool.ntp.org");
    sntp_setservername(2, (char*)"time.nist.gov");
    sntp_set_timezone(0);
    sntp_init();

    // Set callback function to check SNTP timestamp
    os_timer_setfn(&sntp_timer, (os_timer_func_t *) check_sntp_stamp_isr, NULL);
    os_timer_arm(&sntp_timer, SNTP_WAIT_MS, false);

    while ( sntp_synced == false ) {
        delay(500);
        Serial.print(".");
    }

    uint32_t current_stamp = sntp_get_current_timestamp();
    setTime(current_stamp);
    Serial.printf("\nSNTP OK\nTimestamp:       %d,     %s      \n",current_stamp, sntp_get_real_time(current_stamp));

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
 