#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "icmp.h"
#include "ipv4.h"


static uint16_t icmp_checksum(const void*, int);

int main(int argc, char* argv[]){
    if (argc != 2){
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct sockaddr_in dst;
    struct sockaddr_in src;
    struct addrinfo hints;
    struct addrinfo* res;
    struct timeval tv;
    struct timespec t_start, t_end;
    struct ipv4_hdr* ip_hdr;
    struct icmp_hdr* icmp_resp;
    uint8_t ihl_bytes; 
    int sock;
    socklen_t src_len;
    ssize_t n;
    unsigned char recv_buf[1500];
    double rrt; 
    

    struct {
        struct icmp_hdr hdr;
        char data[56]; 
    } pkt;

    // Resolve destination
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol  = IPPROTO_ICMP;
    
    if (getaddrinfo(argv[1], NULL, &hints, &res) != 0){
        perror("getaddrinfo");
        return EXIT_FAILURE;
    }
    dst = *(struct sockaddr_in *)res->ai_addr;
    freeaddrinfo(res);

    // create socket
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0){
        perror("socket (root?)");
        return EXIT_FAILURE;
    }

    // 1sec recieve timeout
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // build icmp request
    memset(&pkt, 0, sizeof(pkt));
    pkt.hdr.type = ICMP_ECHO_REQUEST;
    pkt.hdr.code = 0;
    pkt.hdr.id   = htons((uint16_t)getpid());
    pkt.hdr.seq  = htons(1);
    strcpy(pkt.data, "ping...");

    pkt.hdr.checksum = 0;
    pkt.hdr.checksum = icmp_checksum(&pkt, sizeof(pkt));

    // timestamp before sending
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    if (sendto(sock, &pkt, sizeof(pkt), 0,
                (struct sockaddr *)&dst, sizeof(dst)) < 0){
        perror("sendto");
        close(sock);
        return EXIT_FAILURE;
    }

    // Recieve reply
    src_len = sizeof(src);
    n = recvfrom(sock, recv_buf, sizeof(recv_buf), 0,
            (struct sockaddr *)&src, &src_len);
    if (n < 0){
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            fprintf(stderr, "Request timed out...\n");
        else
            perror("recvfrom");
        close(sock);
        return EXIT_FAILURE;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    rrt = (t_end.tv_sec - t_start.tv_sec) * 1000.0 +
          (t_end.tv_nsec - t_start.tv_nsec) / 1e6;
    
    // locate ICMP part
    ip_hdr = (struct ipv4_hdr *)recv_buf;
    ihl_bytes = ipv4_ihl(ip_hdr) * 4;
    icmp_resp = (struct icmp_hdr *)(recv_buf + ihl_bytes);

    if (icmp_resp->type == ICMP_ECHO_REPLY && 
            icmp_resp->id == pkt.hdr.id) {
        printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
                n - ihl_bytes, inet_ntoa(src.sin_addr), ntohs(icmp_resp->seq),
                ip_hdr->ttl, rrt);
    } else {
        printf("Recieved unexpected ICMP type=%d code=%d\n",
                icmp_resp->type, icmp_resp->code); 
    }

    close(sock);
    return EXIT_SUCCESS;
}


static uint16_t icmp_checksum(const void* buf, int len){
    const uint16_t *data = buf;
    uint32_t sum = 0;

    while (len > 1){
        sum += *data++;
        len -= 2;
    }
    if (len == 1)
        sum += *((const uint8_t *)data);

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)~sum;
}
