#include "packet.h"
#include <string.h>

/*
 * Minimum total packet size: 54 bytes
 * (6 bytes for the primary header and 48 bytes for the application data — 6 x double).
 */
#define MIN_PACKET_LEN 54U

/*
 * read_be_double reads a 64-bit IEEE 754 double from a big-endian byte stream.
 * CCSDS application data uses big-endian (network) byte order, so on little-endian
 * hosts we reverse the bytes before reading the bit pattern as a double.
 * Using memcpy prevents undefined behavior from strict-aliasing.
 */
static double read_be_double(const uint8_t *p)
{
    uint8_t tmp[8];
    tmp[0] = p[7];
    tmp[1] = p[6];
    tmp[2] = p[5];
    tmp[3] = p[4];
    tmp[4] = p[3];
    tmp[5] = p[2];
    tmp[6] = p[1];
    tmp[7] = p[0];

    double value;
    memcpy(&value, tmp, 8);
    return value;
}

ccsds_packet_t parse_packet(const uint8_t *buf, size_t len)
{
    ccsds_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    if (len < MIN_PACKET_LEN) {
        pkt.parse_error = PARSE_ERR_TOO_SHORT;
        return pkt;
    }

    pkt.header.version      = (buf[0] >> 5) & 0x07u;
    pkt.header.packet_type  = (buf[0] >> 4) & 0x01u;
    pkt.header.sec_hdr_flag = (buf[0] >> 3) & 0x01u;

    pkt.header.apid = (uint16_t)(((buf[0] & 0x07u) << 8) | buf[1]);

    pkt.header.seq_flags = (buf[2] >> 6) & 0x03u;
    pkt.header.seq_count = (uint16_t)(((buf[2] & 0x3Fu) << 8) | buf[3]);

    pkt.header.data_length = (uint16_t)((buf[4] << 8) | buf[5]);

    /* must be 000b */
    if (pkt.header.version != 0) {
        pkt.parse_error = PARSE_ERR_VERSION;
        return pkt;
    }

    const uint8_t *data = buf + 6;

    pkt.state.x  = read_be_double(data +  0);
    pkt.state.y  = read_be_double(data +  8);
    pkt.state.z  = read_be_double(data + 16);
    pkt.state.vx = read_be_double(data + 24);
    pkt.state.vy = read_be_double(data + 32);
    pkt.state.vz = read_be_double(data + 40);

    pkt.parse_error = PARSE_OK;
    return pkt;
}
