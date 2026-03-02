#ifndef SIMPLE_IPV4_H
#define SIMPLE_IPV4_H

#include <stdint.h>

struct ipv4_hdr {
    uint8_t version_ihl; 
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
};

// Helper to extract the IHL
static inline uint8_t ipv4_ihl(const struct ipv4_hdr* ip){
    return ip->version_ihl & 0x0F; // low 4 bits
}

#endif
