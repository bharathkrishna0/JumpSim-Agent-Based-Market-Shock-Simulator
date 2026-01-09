#include <stdlib.h>
#include <math.h>
#include <stdint.h>

/*
 * news.c
 * -------
 * Exogenous information arrival process for JumpSim.
 *
 * Economic design principles:
 *  1. News arrivals are rare (Poisson-like)
 *  2. Shock magnitudes are heavy-tailed (fat tails)
 *  3. Regime switching: calm vs stressed markets
 *  4. No artificial price forcing — shocks affect beliefs only
 *
 * This mirrors empirical findings:
 *  - News clustering
 *  - Large tail events (crashes, policy surprises)
 */

/* ---------------- Internal RNG ---------------- */

/* Simple xorshift RNG for reproducibility */
static uint64_t rng_state = 88172645463325252ULL;

static inline uint64_t xorshift64() {
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

static inline double uniform_random() {
    return (xorshift64() >> 11) * (1.0 / 9007199254740992.0);
}

/* ---------------- Regime Model ---------------- */

/*
   Two regimes:
     0 = calm
     1 = stressed / crisis

   Transition probabilities create clustering of volatility.
*/

typedef struct {
    int regime;
    double p_switch_to_stress;
    double p_switch_to_calm;
} NewsRegime;

static NewsRegime regime = {
    .regime = 0,
    .p_switch_to_stress = 0.002,
    .p_switch_to_calm   = 0.01
};

/* ---------------- Heavy-Tail Shock Generator ---------------- */

/*
   Student-t like heavy tail approximation:
   shock = scale * (normal / sqrt(uniform))
*/

static double heavy_tail_shock(double scale) {
    double u = uniform_random();
    if (u < 1e-12) u = 1e-12;

    double z = sqrt(-2.0 * log(u)) * cos(2.0 * M_PI * uniform_random());
    return scale * z / sqrt(uniform_random());
}

/* ---------------- Public API ---------------- */

/*
 * Initialize news generator seed.
 */
void news_seed(uint64_t seed) {
    rng_state = seed;
}

/*
 * Generate one global news shock.
 *
 * Return:
 *   double shock_strength (positive or negative)
 *
 * Interpretation:
 *   - 0.0 => no meaningful news this step
 *   - large magnitude => major macro / sentiment shock
 */

double news_generate_shock() {

    /* -------- Regime switching -------- */

    if (regime.regime == 0) {
        if (uniform_random() < regime.p_switch_to_stress)
            regime.regime = 1;
    } else {
        if (uniform_random() < regime.p_switch_to_calm)
            regime.regime = 0;
    }

    /* -------- Arrival intensity -------- */

    double arrival_prob =
        (regime.regime == 0) ? 0.01 : 0.05;

    if (uniform_random() > arrival_prob) {
        return 0.0;
    }

    /* -------- Shock magnitude -------- */

    double scale =
        (regime.regime == 0) ? 2.0 : 8.0;

    double shock = heavy_tail_shock(scale);

    return shock;
}

/*
 * Return current regime (0 = calm, 1 = stress)
 */
int news_current_regime() {
    return regime.regime;
}
