#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stddef.h>

/*
 * CCSDS Space Packet Primary Header (6 bytes):
 *
 *  Byte 0: [version(3)] [pkt_type(1)] [sec_hdr_flag(1)] [apid_msb(3)]
 *  Byte 1: [apid_lsb(8)]
 *  Byte 2: [seq_flags(2)] [seq_count_msb(6)]
 *  Byte 3: [seq_count_lsb(8)]
 *  Byte 4: [data_length_msb(8)]
 *  Byte 5: [data_length_lsb(8)]
 */

typedef struct {
    uint8_t  version;        /* 3-bit version number, must be 0 */
    uint8_t  packet_type;    /* 0 = telemetry, 1 = telecommand  */
    uint8_t  sec_hdr_flag;   /* secondary header present flag   */
    uint16_t apid;           /* 11-bit application process ID   */
    uint8_t  seq_flags;      /* 2-bit sequence flags            */
    uint16_t seq_count;      /* 14-bit packet sequence count    */
    uint16_t data_length;    /* octets in data field minus one  */
} ccsds_primary_header_t;

/*
 * Orbital state vector: position (m) and velocity (m/s)
 * in an Earth-Centered Inertial  frame.
 */
typedef struct {
    double x, y, z;    /* position components, metres        */
    double vx, vy, vz; /* velocity components, metres/second */
} orbit_state_t;

/*
 * Complete parsed packet.
 * parse_error != 0 means parsing failed... inspect parse_error for the reason.
 */
typedef struct {
    ccsds_primary_header_t header;
    orbit_state_t          state;
    int                    parse_error; /* 0 = success, non-zero = failure */
} ccsds_packet_t;

/* Parse error codes */
#define PARSE_OK            0
#define PARSE_ERR_TOO_SHORT 1   /* buffer shorter than 54 bytes      */
#define PARSE_ERR_VERSION   2   /* version field is not 0            */

/*
 * parse_packet - take a byte buffer and parse a raw CCSDS telemetry packet.
 *
 * @buf: a pointer to the raw byte buffer
 * @len: the number of bytes in the buffer (must be at least 54)
 *
  * Gives back a ccsds_packet_t. The header and state will not be fully filled if parse_error is not zero.
  * fields may be partially filled in and should not be used.
 */
ccsds_packet_t parse_packet(const uint8_t *buf, size_t len);

#endif /* PACKET_H */
