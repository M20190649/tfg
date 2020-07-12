#include "sniffer.hpp"
#include <stdio.h>
#include <MD5Builder.h>
#include <esp_wifi.h>

#define DEBUG 1

std::map<String,uint32_t> Sniffer::sta_detected;

MD5Builder _md5;

String md5(String str) {
    _md5.begin();
    _md5.add(String(str));
    _md5.calculate();

    return _md5.toString();
}

/**
 * Callback for promiscuous mode
 */
void ICACHE_FLASH_ATTR Sniffer::sniffer_callback(void *buff, wifi_promiscuous_pkt_type_t type)
{
    if (type == WIFI_PKT_MGMT) { //Maybe unnecessary after filter
        const wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *)buff;
        struct Sniffer::iee80211_header *header = (struct iee80211_header*) packet->payload;
        if (header->frame_control.type == TYPE_MANAGEMENT 
            && header->frame_control.subtype == SUBTYPE_PROBE_REQUEST) {
              // The second address in MAC header is the transmitting station.
              uint8_t *addr = header->address_2;
              char mac_addr[18];
              sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
              String hashed_mac = md5(mac_addr);
              time_t now;
              time(&now);
              sta_detected[hashed_mac] = now;
              #if DEBUG == 1
                Serial.println();
                struct tm timeinfo;
                localtime_r(&now, &timeinfo);
                Serial.print( "MAC: " + String(mac_addr) + " - Hashed MAC: " + String(hashed_mac) );
                Serial.print(" - Time: ");
                Serial.print(&timeinfo);
                Serial.println();
              #endif
        }
    }
}

/*
 * Change WiFi channel to the next. 
 */
void Sniffer::channel_hop() {
    uint8_t current_channel;
    wifi_second_chan_t wsc = WIFI_SECOND_CHAN_NONE;
    esp_wifi_get_channel(&current_channel, &wsc);
    uint8_t new_channel = (current_channel != 13) ? current_channel+1: 1; 
    esp_wifi_set_channel(new_channel, WIFI_SECOND_CHAN_NONE);
}
