/**
 * \file sniffer.hpp
 *
 * \brief Sniff IEEE 802.11 management packages for ESP8266.
 * 
 * Capture managements packets in all channels (1-13), and get MAC address of 
 * the transmitting station. For ESP8266 chip using Espressif SDK NonOS
 *
 */

#ifndef SNIFFER_HPP_
#define SNIFFER_HPP_

#include <stdint.h>
#include "Arduino.h"
#include <map>

// Expose Espressif SDK functionality
extern "C" {
    #include "user_interface.h"
    #include "sntp.h"
}

namespace Sniffer {

extern std::map<String,uint32_t> sta_detected;///<Store stations detected transmitting probe request.

/**
* Structures for promiscuous mode callback. Taken literally from ESP8226 documentation.
* For more information see ESP8266 Technical reference:
* http://espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf
* If the previous link is broken, try this http://espressif.com/en/support/download/documents
*/

/**
* \struct RxControl
*
* \brief Struct for rx control data provided by SDK API Espressif ESP8266
*
*/
struct RxControl {
    signed rssi:8; // signal intensity of packet
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2; // 0:is not 11n packet; non-0:is 11n packet;
    unsigned legacy_length:12; // if not 11n packet, shows length of packet.
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7; // if is 11n packet, shows the modulation and code used (range from 0 to 76)
    unsigned CWB:1; // if is 11n packet, shows if is HT40 packet or not
    unsigned HT_length:16;// if is 11n packet, shows length of packet.
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1; // if is 11n packet, shows if is LDPC packet or not.
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4; // which channel this packet in.
    unsigned:12;
};

/**
* \struct sniffer_buf
*
* \brief Struct for IEEE 802.11 packets according to Espressif SDK.
*
*/
struct sniffer_buf {
    struct RxControl rx_ctrl;
    uint8_t buf[112];
    uint16_t cnt; // number count of packet
    uint16_t len; // length of packet
};


/************************************************************************************************
*   Structures for IEEE 802.11 MAC frame and header.                                            *
*   For more information see IEEE Std 802.11™-2016 :                                            *
*   http://standards.ieee.org/getieee802/download/802.11-2016.pdf                               *
************************************************************************************************/

// Main types and subtypes used in second and third subfields of frame control
// field of MAC header.
// There are many more types and subtypes defined in the standard, only
// those necessary for the project have been defined.

#define TYPE_MANAGEMENT       0b00
#define TYPE_CONTROL          0b0001
#define TYPE_DATA             0b0010
#define SUBTYPE_PROBE_REQUEST 0b0100

/**
* \struct frame_control
*
* \brief Struct for frame control field of MAC header according to IEEE 802.11 standard.
*
*/
struct frame_control {
    // The first three subfields of the Frame Control field are present for all type
    // and subtypes of frames.
    unsigned protocol_version:2;
    unsigned type:2;
    unsigned subtype:4;
    // There are other fields in frame control depending on the type and subtype fields.
    uint8_t stuff;
} __attribute__((packed)); // Disable structure alignment padding, this avoid shifting
                           // the data of the rest of the header.

/**
* \struct ieee80211_header
*
* \brief Struct for IEEE 802.11 MAC header.
*
*/
struct iee80211_header {
    // The first three subfields are present in all frames.
    struct frame_control frame_control;
    uint16_t duration_id;
    uint8_t address_1[6];
    // The next three fields are present in all management frame subtypes.
    uint8_t address_2[6];
    uint8_t address_3[6];
    uint16_t seq_ctrl;
    // There are other fields defined in the standard, not necessary for the 
    // purpose of this project. For more information see IEEE 802.11 Standard
};

/**
 * \brief RX callback function for process packets received in promiscuous mode.
 *
 * Check if packet received in buff is a probe request package , if it is
 * extracts the MAC address of the transmitting station.
 *
 * \param[in]     buffer the data received.
 * \param[out]    length data length
 *
 */
void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length);

/**
 * \brief Change WiFi channel to the next.
 *
 * Increment the actual WiFi channel to the next until channel 13, when run out
 * start again from first channel.
 *
 */
void channel_hop();
}
#endif
