/**
 * Radar
 *
 * Scan for wifi devices.
 *
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <MD5Builder.h>
#include <sniffer.hpp>
#include <WiFi.h>

#define DEBUG 0

#define WIFI_SSID ""
#define WIFI_PASSWD ""
#define SERVER_URL ""

String station_id; // ID of the esp32, generated in setup with hashed mac (md5).
BLEScan* p_ble_scan;
std::map<String,uint32_t> ble_detected;
bool ble_scan_active = false;

class ble_advertised_device_cb: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        time_t now;
        time(&now);
        String bd_addr = advertisedDevice.getAddress().toString().c_str();
        ble_detected[bd_addr] = now;

        #if DEBUG == 1
        Serial.println();
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        Serial.printf("Bluetooth detection: | MAC: %s ", advertisedDevice.getAddress().toString().c_str());
        Serial.print(" - Time: ");
        Serial.print(&timeinfo);
        Serial.println();
        #endif
    }
};

String md5(String str) {
    MD5Builder _md5;

    _md5.begin();
    _md5.add(String(str));
    _md5.calculate();

    return _md5.toString();
}

void connect_wifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
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

void promiscuous_mode(bool enable=true) {
    if (enable) {
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
    } else {
        esp_wifi_set_promiscuous(false);
        esp_wifi_stop();
    }

}

void send_detections() {
    Serial.println("Sending detections. ");
    connect_wifi();

    // Memory pool for JSON object tree (bytes).
    DynamicJsonDocument json_doc(20000);

    json_doc["node"] = station_id;
    JsonObject detections = json_doc.createNestedObject("detections");
    for (std::map<String,uint32_t>::iterator it=Sniffer::sta_detected.begin(); it!=Sniffer::sta_detected.end(); ++it) {
        detections[md5(it->first)] = it->second;
    }
    for (std::map<String,uint32_t>::iterator it=ble_detected.begin(); it!=ble_detected.end(); ++it) {
        detections[md5(it->first)] = it->second;
    }

    // Clear detecions
    Sniffer::sta_detected.clear();
    ble_detected.clear();

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
}

void ICACHE_FLASH_ATTR setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(5);
    Serial.println();
    Serial.println("Initializing system.");
    Serial.println("Generating station ID.");
    
    // Generate station ID as MAC hashed md5
    station_id = md5(WiFi.macAddress());
    Serial.printf("Station_id: %s \n", station_id.c_str());

    // WiFi connection
    connect_wifi();

    //Get time through SNTP
    Serial.print("Getting timestamp from server.");

    const char* ntp_server1 = "hora.roa.es";
    const char* ntp_server2 = "pool.ntp.org";
    const char* ntp_server3 = "time.nist.gov";
    const long  gmt_offset_sec = 3600;
    const int   daylight_offset_sec = 3600;
    
    configTime(gmt_offset_sec, daylight_offset_sec, ntp_server1, ntp_server2, ntp_server3); 

    struct tm timeinfo;
    while ( !getLocalTime(&timeinfo) ) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(&timeinfo);

    Serial.println("Starting sniffer mode.");
    
    BLEDevice::init("");
    p_ble_scan = BLEDevice::getScan();
    p_ble_scan->setAdvertisedDeviceCallbacks(new ble_advertised_device_cb());
    p_ble_scan->setActiveScan(true);
    p_ble_scan->setInterval(100);
    p_ble_scan->setWindow(99);
    
    promiscuous_mode(true);
}

void scan_complete_cb(BLEScanResults scanResults) {
	ble_scan_active = false;
}

void loop() {
    uint32_t detecions_count = Sniffer::sta_detected.size() + ble_detected.size();
    if ( detecions_count >= 100) {
        p_ble_scan->stop();
        promiscuous_mode(false);
        send_detections();
        p_ble_scan->start(5, scan_complete_cb, false);
        promiscuous_mode(true);
    }
    if (!ble_scan_active) {
        p_ble_scan->start(5, scan_complete_cb, false);
        ble_scan_active = true;
    }
    Sniffer::channel_hop();
    delay(200);
}
