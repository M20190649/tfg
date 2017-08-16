#include "sniffer.hpp"
#include "Arduino.h"
#include <stdio.h>
#include <string>

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
void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length)
{
    if (length == 128) { // == sizeof (struct sniffer_buf). 
        //Management package. It has 112 Bytes data.
        struct sniffer_buf *packet = (struct sniffer_buf*) buffer;
        struct iee80211_header *header = (struct iee80211_header*) packet->buf;
        if (header->frame_control.type == TYPE_MANAGEMENT 
            && header->frame_control.subtype == SUBTYPE_PROBE_REQUEST) {
              // The second address in MAC header is the transmitting station.
              uint8_t *addr = header->address_2;
              char mac_addr[18];
              sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
              Serial.println();
              Serial.print(md5(mac_addr));
        }
    }
}

/*
 * Change WiFi channel to the next. 
 */
void channel_hop() {
    uint8_t current_channel = wifi_get_channel();
    uint8_t new_channel = (current_channel != 13) ? current_channel+1: 1; 
    wifi_set_channel(new_channel);
}
