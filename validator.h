#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "packet.h"

/*
 * Validation error codes:
 * 0 VALID - all checks were successful
 * 1 ALTITUDE_RANGE: position magnitude is outside of [6521000, 42164000] m
 * 2 VELOCITY_BAD: The speed is outside of [50%, 200%] of the circular speed.
 * 3 UNBOUND_ORBIT means that the energy per unit mass in the orbit is greater than or equal to 0 (escape trajectory).
 */
#define VALID           0
#define ERR_ALTITUDE    1
#define ERR_VELOCITY    2
#define ERR_UNBOUND     3

/* Maximum human-readable message length */
#define VALIDATOR_MSG_LEN 256

typedef struct {
    int  passed;               /* 1 = orbit is plausible, 0 = failed    */
    int  error_code;           /* one of the codes above                */
    char message[VALIDATOR_MSG_LEN]; /* human-readable description      */
} validation_result_t;

/*
 * validate_orbit - check the physical plausibility of an orbital state vector.
 *
 * @state : a pointer to a fully parsed orbit_state_t
 *
 * Returns a validation_result_t that tells you what happened.
 */
validation_result_t validate_orbit(const orbit_state_t *state);

#endif /* VALIDATOR_H */

