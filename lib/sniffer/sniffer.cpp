#include "sniffer.hpp"
#include <stdio.h>
#include <Time.h>

#define DEBUG 0

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
void ICACHE_FLASH_ATTR Sniffer::sniffer_callback(uint8_t *buffer, uint16_t length)
{
    static const char MAC_FMT[] = "%02X:%02X:%02X:%02X:%02X:%02X"; ///< MAC address format for printf

    if (length == 128) { // == sizeof (struct sniffer_buf).
        //Management package. It has 112 Bytes data.
        struct Sniffer::sniffer_buf *packet = (struct sniffer_buf*) buffer;
        struct Sniffer::iee80211_header *header = (struct iee80211_header*) packet->buf;
        if (header->frame_control.type == TYPE_MANAGEMENT 
            && header->frame_control.subtype == SUBTYPE_PROBE_REQUEST) {
              // The second address in MAC header is the transmitting station.
              uint8_t *addr = header->address_2;
              char mac_addr[18];
              sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
              String hashed_mac = md5(mac_addr);
              sta_detected[hashed_mac] = now();
              #if DEBUG == 1
                Serial.println();
                Serial.print(hashed_mac);
                Serial.printf("  Time:%s\n", sntp_get_real_time(sta_detected[hashed_mac]));
              #endif
        }
    }
}

/*
 * Change WiFi channel to the next. 
 */
void Sniffer::channel_hop() {
    uint8_t current_channel = wifi_get_channel();
    uint8_t new_channel = (current_channel != 13) ? current_channel+1: 1; 
    wifi_set_channel(new_channel);
}
