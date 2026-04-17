#include "validator.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* --------------------------- Physical constants --------------------------- */

/* Earth's gravitational parameter mu = GM [m^3/s^2] */
#define MU          3.986004418e14

/* Earth's mean radius [m] */ 
#define EARTH_RADIUS_M  6371000.0

/* Minimum altitude: ~150 km above surface means r_min = 6521 km */
#define R_MIN_M         6521000.0

/* Maximum altitude: geostationary orbit radius ~42,164 km */ 
#define R_MAX_M         42164000.0

/* Velocity tolerance: accept [50%, 200%] of circular speed */
#define V_RATIO_MIN     0.50
#define V_RATIO_MAX     2.00

/* --------------------------- validate_orbit --------------------------- */
validation_result_t validate_orbit(const orbit_state_t *state)
{
    validation_result_t result;
    memset(&result, 0, sizeof(result));

    double r = sqrt(state->x * state->x +
                    state->y * state->y +
                    state->z * state->z);

    if (r < R_MIN_M || r > R_MAX_M) {
        result.passed     = 0;
        result.error_code = ERR_ALTITUDE;
        snprintf(result.message, VALIDATOR_MSG_LEN,
                 "ALTITUDE OUT OF RANGE: position magnitude = %.3f m "
                 "(altitude = %.3f km). "
                 "Acceptable range: [%.0f m, %.0f m].",
                 r,
                 (r - EARTH_RADIUS_M) / 1000.0,
                 R_MIN_M,
                 R_MAX_M);
        return result;
    }
    
    double v = sqrt(state->vx * state->vx +
                    state->vy * state->vy +
                    state->vz * state->vz);

    /* this radius' circular orbital speed: v_c = sqrt(mu / r) */
    double v_circular = sqrt(MU / r);

    double v_lo = V_RATIO_MIN * v_circular;
    double v_hi = V_RATIO_MAX * v_circular;

    if (v < v_lo || v > v_hi) {
        result.passed     = 0;
        result.error_code = ERR_VELOCITY;
        snprintf(result.message, VALIDATOR_MSG_LEN,
                 "VELOCITY IMPLAUSIBLE: velocity magnitude = %.3f m/s, "
                 "circular speed at this altitude = %.3f m/s. "
                 "Acceptable range: [%.3f m/s, %.3f m/s].",
                 v, v_circular, v_lo, v_hi);
        return result;
    }

    /* E = v^2/2 - mu/r */
    double energy = (v * v) / 2.0 - MU / r;

    if (energy >= 0.0) {
        result.passed     = 0;
        result.error_code = ERR_UNBOUND;
        snprintf(result.message, VALIDATOR_MSG_LEN,
                 "UNBOUND ORBIT: specific orbital energy = %.6e J/kg "
                 "(must be < 0 for a bound orbit). "
                 "Spacecraft appears to be on an escape or hyperbolic trajectory.",
                 energy);
        return result;
    }

    result.passed     = 1;
    result.error_code = VALID;
    snprintf(result.message, VALIDATOR_MSG_LEN,
             "VALID: position magnitude = %.3f m (altitude = %.3f km), "
             "velocity = %.3f m/s (circular ref = %.3f m/s), "
             "specific energy = %.6e J/kg.",
             r,
             (r - EARTH_RADIUS_M) / 1000.0,
             v,
             v_circular,
             energy);
    return result;
}
