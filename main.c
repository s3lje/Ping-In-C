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

}

static uint16_t icmp_checksum(const void* buf, int len){

}
