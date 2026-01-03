#include "agent.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ----------------------------------------------------
   Internal utilities (private to agent.c)
---------------------------------------------------- */

/* Simple Box–Muller normal generator (mean 0, std 1) */
static double normal_random(uint64_t *state) {
    /* xorshift64 for reproducible RNG */
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;

    double u1 = (x & 0xFFFFFFFF) / (double)0xFFFFFFFF;
    double u2 = ((x >> 32) & 0xFFFFFFFF) / (double)0xFFFFFFFF;

    if (u1 < 1e-12) u1 = 1e-12;

    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

/* ----------------------------------------------------
   Initialization
---------------------------------------------------- */

void agent_init(Agent *a,
                AgentId id,
                AgentType type,
                const char *name,
                double init_price,
                double aggressiveness,
                double trade_size_scale,
                double risk_aversion,
                double liquidity_tolerance,
                double belief_update_rate,
                double network_influence,
                double noise_std,
                double fundamental_anchor,
                uint64_t rng_seed)
{
    memset(a, 0, sizeof(Agent));

    a->id = id;
    a->type = type;
    strncpy(a->name, name, AGENT_NAME_MAX - 1);

    /* Initial belief seeded at market price */
    a->belief = init_price;
    a->belief_update_rate = belief_update_rate;

    a->aggressiveness = aggressiveness;
    a->trade_size_scale = trade_size_scale;
    a->risk_aversion = risk_aversion;
    a->liquidity_tolerance = liquidity_tolerance;

    a->network_influence = network_influence;
    a->neighbors = NULL;
    a->neighbor_count = 0;

    a->position = 0;
    a->cash = 0.0;

    a->noise_std = noise_std;
    a->fundamental_anchor = fundamental_anchor;

    a->rng_state = rng_seed;
    a->passive_only = false;
}

/* ----------------------------------------------------
   Demand computation
---------------------------------------------------- */

double agent_compute_demand(const Agent *a,
                            double market_price,
                            double global_shock,
                            double avg_neighbor_belief)
{
    /*
     Economic decomposition of demand:

     demand =
        α (belief − price)               → valuation / momentum signal
      − β * inventory_penalty(position)  → risk control
      + γ (neighbor_belief − belief)     → herding
      + ε                                → idiosyncratic noise
    */

    /* 1. Price signal */
    double signal = a->belief - market_price;

    /* Institutions anchor to fundamentals */
    if (a->type == AGENT_INSTITUTION) {
        signal += 0.5 * (a->fundamental_anchor - market_price);
    }

    /* 2. Inventory risk penalty */
    double inventory_cost = a->risk_aversion * position_penalty(a->position);

    /* 3. Herding / network influence */
    double herding = 0.0;
    if (a->neighbor_count > 0) {
        herding = a->network_influence * (avg_neighbor_belief - a->belief);
    }

    /* 4. Noise */
    double noise = a->noise_std * normal_random((uint64_t *)&a->rng_state);

    /* Combine */
    double raw_demand =
        a->aggressiveness * signal
        - inventory_cost
        + herding
        + noise
        + global_shock;

    /* Liquidity threshold: avoid micro trades */
    if (fabs(raw_demand) < a->liquidity_tolerance) {
        return 0.0;
    }

    /* Scale into trade units */
    return a->trade_size_scale * raw_demand;
}

/* ----------------------------------------------------
   Execution update
---------------------------------------------------- */

void agent_apply_execution(Agent *a, int executed_quantity, double execution_price)
{
    /*
     Accounting identity:
     Δcash + price * Δposition = 0
    */

    a->position += executed_quantity;
    a->cash -= executed_quantity * execution_price;
}

/* ----------------------------------------------------
   Belief update
---------------------------------------------------- */

void agent_update_belief(Agent *a,
                         double observed_price,
                         double global_shock,
                         double avg_market_signal)
{
    /*
     Adaptive expectations:
     belief(t+1) = belief(t)
                 + λ [ observed_price − belief(t) ]
                 + shock_component
    */

    double target = observed_price;

    /* Institutions filter noise more aggressively */
    if (a->type == AGENT_INSTITUTION) {
        target = 0.7 * observed_price + 0.3 * a->fundamental_anchor;
    }

    a->belief += a->belief_update_rate * (target - a->belief);
    a->belief += 0.1 * global_shock;
}

/* ----------------------------------------------------
   Shock response
---------------------------------------------------- */

void agent_apply_shock(Agent *a, double shock_strength)
{
    /*
     Heterogeneous reaction to news:
     - Retail: overreact
     - Institution: dampened response
     - Noise: random
    */

    if (a->type == AGENT_RETAIL) {
        a->belief += 1.2 * shock_strength;
    }
    else if (a->type == AGENT_INSTITUTION) {
        a->belief += 0.4 * shock_strength;
    }
    else {
        a->belief += shock_strength * normal_random(&a->rng_state);
    }
}

/* ----------------------------------------------------
   Serialization (optional but impressive)
---------------------------------------------------- */

char *agent_to_json(const Agent *a)
{
    char *buf = malloc(512);
    if (!buf) return NULL;

    snprintf(buf, 512,
        "{ \"id\": %u, \"type\": %d, \"belief\": %.4f, "
        "\"position\": %d, \"cash\": %.4f }",
        a->id, a->type, a->belief, a->position, a->cash
    );

    return buf;
}

/* ----------------------------------------------------
   Cleanup
---------------------------------------------------- */

void agent_free(Agent *a)
{
    /* Neighbors are owned by simulation controller */
    a->neighbors = NULL;
    a->neighbor_count = 0;
}
