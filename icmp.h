#ifndef SIMPLE_ICMP_H
#define SIMPLE_ICMP_H

#include <stdint.h>

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY   0

struct icmp_hdr{
    uint8_t type;       // 8 = echo request, 0 = echo reply
    uint8_t code;       // always 0 for echo
    uint16_t checksum;  // computed over header and payload
    uint16_t id;        // usually pid
    uint16_t seq;       // increments per packet
};

#endif
