/**
 * Radar
 *
 * Scan for wifi devices.
 *
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <sniffer.hpp>
#include <MD5Builder.h>
#include <esp_wifi.h>

#define DEBUG 1

#define WIFI_SSID "Lowi2A18"
#define WIFI_PASSWD "Sql1D00WTF."
#define SERVER_URL "http://192.168.0.13:5000/api/v1.0/detections-collection/"

char station_id[32]; // ID of the esp32, generated in setup with hashed mac (md5).

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

void promiscuous_mode() {
    WiFi.disconnect(true);
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_cfg);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    const wifi_promiscuous_filter_t filter={.filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT};
    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_promiscuous_rx_cb(&Sniffer::sniffer_callback);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
}

void send_detections() {
    if (Sniffer::sta_detected.size() > 0 ) {
        Serial.println("Sending detections. ");
        esp_wifi_set_promiscuous(false);
        esp_wifi_stop();
        delay(50);
        connect_wifi();

        // Memory pool for JSON object tree (bytes).
        DynamicJsonDocument json_doc(20000);

        json_doc["node"] = station_id;
        JsonObject detections = json_doc.createNestedObject("detections");

        for (std::map<String,uint32_t>::iterator it=Sniffer::sta_detected.begin(); it!=Sniffer::sta_detected.end(); ++it){
            detections[it->first] = it->second;
        }

        // Clear detecions
        Sniffer::sta_detected.clear();

        #if DEBUG == 1
        serializeJsonPretty(json_doc, Serial);
        Serial.println();
        #endif

        String databuf;
        serializeJson(json_doc, databuf);
        // Post detections
        HTTPClient http;
        http.begin(SERVER_URL);
        http.addHeader("Content-Type", "application/json");
        http.POST(databuf);
        http.end();

        promiscuous_mode();
    } else {
        Serial.println("No detections.");
    }
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

    // WiFi connection
    connect_wifi();

    //Get time through SNTP
    Serial.print("Getting timestamp from server.");

    
    const char* ntp_server1 = "hora.roa.es";
    const char* ntp_server2 = "pool.ntp.org";
    const char* ntp_server3 = "time.nist.gov";
    const long  gmtOffset_sec = 3600;
    const int   daylightOffset_sec = 3600;
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntp_server1, ntp_server2, ntp_server3);

    struct tm timeinfo;
    while ( !getLocalTime(&timeinfo) ) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(&timeinfo);

    
    Serial.println("Starting sniffer mode.");
    promiscuous_mode();
}

void loop()
{
    uint8_t current_channel;
    wifi_second_chan_t wsc = WIFI_SECOND_CHAN_NONE;
    esp_wifi_get_channel(&current_channel, &wsc);
    if (current_channel == 13) {
        send_detections();
    }
    
    Sniffer::channel_hop();
    delay(500);
}
 