#include <cstring>
#include <iostream>
#include <pcap.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network/ArpPacket.h"

#define MAX_BYTES 2048

using namespace network;
using namespace std;

HwAddr get_dev_mac(const char *dev) {
    ifreq ifr{};
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    strncpy(ifr.ifr_name, dev, IF_NAMESIZE-1);

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
        close(fd);
        return HwAddr("00:00:00:00:00:00");
    }

    byte mac[6];
    memcpy(mac, ifr.ifr_addr.sa_data, HwAddr::size);

    close(fd);
    return HwAddr(mac);
}

int main(int argc, char *argv[]) {
    pcap_t *desc = nullptr;
    char errbuf[PCAP_ERRBUF_SIZE] = {};
    char *dev = argv[1];

    std::memset(errbuf, 0, PCAP_ERRBUF_SIZE);

    cout << "open device " << dev << endl;

    if ((desc = pcap_open_live(dev, MAX_BYTES, 0, 512, errbuf)) == nullptr) {
        cout << "error: "  << errbuf << endl;
        return -1;
    }

    ArpPacket arp(ARPOP_REPLY,
            get_dev_mac(dev), IpAddr(argv[2]),
            HwAddr("BB:BB:BB:BB:BB:BB"), IpAddr(argv[3]));
    byte *packet = arp.to_bytes();

    if (pcap_sendpacket(desc, packet, ArpPacket::size) != 0) {
        fprintf(stderr, "\nError sending the packet : %s\n", pcap_geterr(desc));
        exit(EXIT_FAILURE);
    }

    free(packet);
    pcap_close(desc);
    return 0;
}