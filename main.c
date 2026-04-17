#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "packet.h"
#include "validator.h"

/*
 * build_packet_bytes : construct a minimal CCSDS telemetry packet in memory.
 *
 * Header (6 bytes):
 *   version=0, pkt_type=0 (TM), sec_hdr_flag=0, apid=0x64 (100)
 *   seq_flags=3 (standalone), seq_count=1
 *   data_length=47  (48 bytes of app data minus 1)
 *
 * Application data (48 bytes):
 *   ISS-like LEO state vector (ECI frame, J2000 epoch):
 *     x  =  6778000.0  m   (~407 km altitude)
 *     y  =        0.0  m
 *     z  =        0.0  m
 *     vx =        0.0  m/s
 *     vy =     7660.0  m/s  (typical LEO circular speed)
 *     vz =        0.0  m/s
 */
static void write_be_double(uint8_t *dest, double value)
{
    uint8_t raw[8];
    memcpy(raw, &value, 8);
    /* Reverse bytes: host (little-endian assumed) -> big-endian */
    dest[0] = raw[7];
    dest[1] = raw[6];
    dest[2] = raw[5];
    dest[3] = raw[4];
    dest[4] = raw[3];
    dest[5] = raw[2];
    dest[6] = raw[1];
    dest[7] = raw[0];
}

static void build_packet_bytes(uint8_t *buf)
{
    uint16_t apid      = 100;       /* 0x064                          */
    uint8_t  seq_flags = 0x3;       /* 11b = standalone packet        */
    uint16_t seq_count = 1;
    uint16_t data_len  = 47;        /* 48 bytes of data - 1           */

    buf[0] = (uint8_t)((0x00u << 5) |      /* version  = 0   */
                       (0x00u << 4) |      /* pkt_type = 0   */
                       (0x00u << 3) |      /* sec_hdr  = 0   */
                       ((apid >> 8) & 0x07u));

    buf[1] = (uint8_t)(apid & 0xFFu);

    buf[2] = (uint8_t)((seq_flags << 6) | ((seq_count >> 8) & 0x3Fu));

    buf[3] = (uint8_t)(seq_count & 0xFFu);

    buf[4] = (uint8_t)((data_len >> 8) & 0xFFu);
    buf[5] = (uint8_t)(data_len & 0xFFu);

    /* 6 doubles in big-endian order */
    write_be_double(buf +  6,  6778000.0); /* x  */
    write_be_double(buf + 14,        0.0); /* y  */
    write_be_double(buf + 22,        0.0); /* z  */
    write_be_double(buf + 30,        0.0); /* vx */
    write_be_double(buf + 38,     7660.0); /* vy */
    write_be_double(buf + 46,        0.0); /* vz */
}

int main(void)
{
    uint8_t buf[54];
    build_packet_bytes(buf);

    printf("=== ccsds-orbit-validator demo ===\n\n");

    ccsds_packet_t pkt = parse_packet(buf, sizeof(buf));

    if (pkt.parse_error != PARSE_OK) {
        printf("PARSE ERROR %d\n", pkt.parse_error);
        if (pkt.parse_error == PARSE_ERR_TOO_SHORT)
            printf("  Reason: buffer shorter than 54 bytes.\n");
        else if (pkt.parse_error == PARSE_ERR_VERSION)
            printf("  Reason: CCSDS version field is not 0.\n");
        return 1;
    }

    printf("--- Parsed Primary Header ---\n");
    printf("  Version          : %u\n",  pkt.header.version);
    printf("  Packet Type      : %u  (0=TM)\n", pkt.header.packet_type);
    printf("  Sec Header Flag  : %u\n",  pkt.header.sec_hdr_flag);
    printf("  APID             : %u\n",  pkt.header.apid);
    printf("  Sequence Flags   : %u\n",  pkt.header.seq_flags);
    printf("  Sequence Count   : %u\n",  pkt.header.seq_count);
    printf("  Data Length      : %u\n",  pkt.header.data_length);

    printf("\n--- Parsed Orbital State Vector ---\n");
    printf("  Position  x = %12.3f m\n", pkt.state.x);
    printf("  Position  y = %12.3f m\n", pkt.state.y);
    printf("  Position  z = %12.3f m\n", pkt.state.z);
    printf("  Velocity vx = %12.3f m/s\n", pkt.state.vx);
    printf("  Velocity vy = %12.3f m/s\n", pkt.state.vy);
    printf("  Velocity vz = %12.3f m/s\n", pkt.state.vz);

    validation_result_t res = validate_orbit(&pkt.state);

    printf("\n--- Validation Result ---\n");
    printf("  Status     : %s\n", res.passed ? "PASS" : "FAIL");
    printf("  Error code : %d\n", res.error_code);
    printf("  Message    : %s\n", res.message);

    return res.passed ? 0 : 1;
}
