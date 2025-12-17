#include "services/dns_resolver.h"
#include <WiFiUdp.h>

namespace dns_resolver {
    static bool skip_name(const uint8_t* buf, int len, int& off) {
        if (off >= len) return false;
        while (off < len) {
            uint8_t v = buf[off];
            if (v == 0) {
                off += 1;
                return true;
            }
            if ((v & 0xC0) == 0xC0) {
                if (off + 1 >= len) return false;
                off += 2;
                return true;
            }
            int l = v;
            if (off + 1 + l > len) return false;
            off += 1 + l;
        }
        return false;
    }

    static bool parse_response(uint8_t* buf, int len, IPAddress& out) {
        if (len < 12) return false;
        uint16_t qdcount = (uint16_t(buf[4]) << 8) | buf[5];
        uint16_t ancount = (uint16_t(buf[6]) << 8) | buf[7];
        int off = 12;
        for (uint16_t i = 0; i < qdcount; ++i) {
            if (!skip_name(buf, len, off)) return false;
            if (off + 4 > len) return false;
            off += 4;
        }
        for (uint16_t i = 0; i < ancount; ++i) {
            if (!skip_name(buf, len, off)) return false;
            if (off + 10 > len) return false;
            uint16_t type = (uint16_t(buf[off]) << 8) | buf[off + 1];
            uint16_t klass = (uint16_t(buf[off + 2]) << 8) | buf[off + 3];
            off += 4;
            uint32_t ttl = (uint32_t(buf[off]) << 24) | (uint32_t(buf[off + 1]) << 16) | (uint32_t(buf[off + 2]) << 8) | uint32_t(buf[off + 3]);
            (void)ttl;
            off += 4;
            if (off + 2 > len) return false;
            uint16_t rdlen = (uint16_t(buf[off]) << 8) | buf[off + 1];
            off += 2;
            if (off + rdlen > len) return false;
            if (type == 1 && klass == 1 && rdlen == 4) {
                out = IPAddress(buf[off], buf[off + 1], buf[off + 2], buf[off + 3]);
                return true;
            }
            off += rdlen;
        }
        return false;
    }

    bool resolve(const char* host, IPAddress& out) {
        if (!host || !*host) return false;
        WiFiUDP udp;
        if (!udp.begin(0)) return false;
        uint8_t buf[256];
        uint16_t id = uint16_t(millis());
        buf[0] = uint8_t(id >> 8);
        buf[1] = uint8_t(id & 0xFF);
        buf[2] = 0x01;
        buf[3] = 0x00;
        buf[4] = 0x00;
        buf[5] = 0x01;
        buf[6] = 0x00;
        buf[7] = 0x00;
        buf[8] = 0x00;
        buf[9] = 0x00;
        buf[10] = 0x00;
        buf[11] = 0x00;
        int off = 12;
        const char* p = host;
        while (*p) {
            const char* dot = strchr(p, '.');
            size_t label_len = dot ? size_t(dot - p) : strlen(p);
            if (label_len == 0 || label_len > 63) {
                udp.stop();
                return false;
            }
            if (off + 1 + int(label_len) + 2 > int(sizeof(buf))) {
                udp.stop();
                return false;
            }
            buf[off++] = uint8_t(label_len);
            memcpy(buf + off, p, label_len);
            off += int(label_len);
            if (!dot) {
                buf[off++] = 0;
                break;
            }
            p = dot + 1;
        }
        if (buf[off - 1] != 0) {
            if (off + 1 > int(sizeof(buf))) {
                udp.stop();
                return false;
            }
            buf[off++] = 0;
        }
        if (off + 4 > int(sizeof(buf))) {
            udp.stop();
            return false;
        }
        buf[off++] = 0;
        buf[off++] = 1;
        buf[off++] = 0;
        buf[off++] = 1;
        int query_len = off;
        IPAddress servers[2] = { IPAddress(223, 5, 5, 5), IPAddress(1, 1, 1, 1) };
        for (int si = 0; si < 2; ++si) {
            unsigned long start = millis();
            udp.flush();
            udp.beginPacket(servers[si], 53);
            udp.write(buf, query_len);
            udp.endPacket();
            while (millis() - start < 1500) {
                int packetSize = udp.parsePacket();
                if (packetSize <= 0) {
                    delay(10);
                    continue;
                }
                int len = udp.read(buf, sizeof(buf));
                if (len <= 0) continue;
                if (parse_response(buf, len, out)) {
                    udp.stop();
                    return true;
                }
            }
        }
        udp.stop();
        return false;
    }
}

