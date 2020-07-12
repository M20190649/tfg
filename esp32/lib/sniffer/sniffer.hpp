/**
 * \file sniffer.hpp
 *
 * \brief Sniff IEEE 802.11 management packages for ESP32.
 * 
 * Capture managements packets in all channels (1-13), and get MAC address of 
 * the transmitting station. For ESP32 chip using Espressif esp-idf framework.
 *
 */

#ifndef SNIFFER_HPP_
#define SNIFFER_HPP_

#include <stdint.h>
#include "Arduino.h"
#include <map>
#include <esp_wifi.h>


namespace Sniffer {

extern std::map<String,uint32_t> sta_detected;///<Store stations detected transmitting probe request.

/**
* Structures for promiscuous mode callback.
*/

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
    unsigned char payload[];
    // There are other fields defined in the standard, not necessary for the 
    // purpose of this project. For more information see IEEE 802.11 Standard
};

/**
 * \brief RX callback function for process packets received in promiscuous mode.
 *
 * Check if packet received in buff is a probe request package , if it is
 * extracts the MAC address of the transmitting station.
 *
 * \param[in]    Data received. Type of data in buffer (wifi_promiscuous_pkt_t or wifi_pkt_rx_ctrl_t) indicated by 'type' parameter.
 * \param[in]    promiscuous packet type.
 *
 */
void ICACHE_FLASH_ATTR sniffer_callback(void *buff, wifi_promiscuous_pkt_type_t type);

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
