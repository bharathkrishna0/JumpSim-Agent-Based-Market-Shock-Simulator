#ifndef JUMPSIM_AGENT_H
#define JUMPSIM_AGENT_H

/*
 * agent.h
 * --------
 * Agent representation for JumpSim (research-grade).
 *
 * Design goals:
 *  - Explicit micro-foundations (beliefs, risk aversion, herding)
 *  - Small public API (init, decide, update, apply execution)
 *  - Clear memory ownership (who allocates / frees neighbor lists)
 *  - Easy to instrument and serialize for experiments
 *
 * Economic notes:
 *  - Demand (units) is computed from a linear combination of:
 *      (1) belief - market_price  (fundamental/momentum signal)
 *      (2) current inventory * risk_aversion (inventory cost)
 *      (3) network influence (herding)
 *      (4) idiosyncratic noise
 *
 *  - This header intentionally does not encode the exact formula; implement
 *    that in agent.c so the experiment code can be varied and tested.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* -------------------- Types & Constants -------------------- */

#define AGENT_NAME_MAX 32

typedef enum {
    AGENT_RETAIL = 0,      /* momentum-prone, high herding */
    AGENT_INSTITUTION = 1, /* fundamental, risk-aware, large-capacity */
    AGENT_NOISE = 2        /* liquidity/noise provider */
} AgentType;

/* Agent identifier type */
typedef uint32_t AgentId;

/* Public Agent structure (opaque-ish but fields are exposed for analysis) */
typedef struct Agent {
    /* Identity & bookkeeping */
    AgentId id;
    char name[AGENT_NAME_MAX]; /* short label for debug / logging */
    AgentType type;

    /* Beliefs & learning */
    double belief;            /* agent's subjective expected price (units: same as market price) */
    double belief_update_rate;/* [0,1] how fast belief tracks observed price (small -> slow learner) */

    /* Trading behavior parameters */
    double aggressiveness;    /* scales demand from signal (higher -> larger orders) */
    double trade_size_scale;  /* base lot size multiplier */
    double risk_aversion;     /* penalizes large inventory (higher -> smaller positions) */
    double liquidity_tolerance;/* minimum price move required to trade (microstructure threshold) */

    /* Herding / network influence */
    double network_influence; /* weight placed on neighbors' average belief (0 = independent) */
    int *neighbors;           /* array of neighbor AgentId's (optional), NULL if none */
    size_t neighbor_count;    /* count of neighbors; caller owns memory for this array */

    /* Inventory & wealth (stateful) */
    int position;             /* signed integer share count (positive = long) */
    double cash;              /* cash holdings (for P&L and risk checks) */

    /* Noise & idiosyncratic terms */
    double noise_std;         /* standard deviation for idiosyncratic noise (normal) */

    /* Meta (for reproducibility and diagnostics) */
    uint64_t rng_state;       /* optional, per-agent RNG state (if you implement xorshift/etc.) */
    double fundamental_anchor;/* long-run value anchor used by institutionals (if any) */

    /* Flags for algorithmic control */
    bool passive_only;        /* if true, the agent only supplies liquidity (no market orders) */
} Agent;

/* -------------------- API (functions to implement in agent.c) -------------------- */

/*
 * Initialize an agent struct (deterministic fields).
 * - 'a' must point to allocated memory.
 * - Leaves neighbor pointer as NULL (caller may set neighbors manually).
 *
 * Parameters explanation:
 *  - init_price: initial market price for seeding belief
 *  - aggressiveness, risk_aversion, etc: sensible defaults are recommended
 */
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
                uint64_t rng_seed);

/*
 * Compute desired (signed) demand for this agent in **units of asset** (positive => buy).
 *
 * Arguments:
 *  - a: agent (const)
 *  - market_price: current market price
 *  - global_shock: external information shock (can be 0)
 *  - avg_neighbor_belief: precomputed mean belief of agent's neighbors (or 0 if none)
 *
 * Return:
 *  - signed double representing desired change in position (fractional allowed;
 *    rounding/execution handled by matching engine)
 *
 * Behavioral form (example documented logic used in implementation):
 *  demand = aggressiveness * (a->belief - market_price)
 *         - risk_aversion * position_penalty(a->position)
 *         + network_influence * (avg_neighbor_belief - a->belief)
 *         + normal_noise(0, noise_std)
 *
 * Implementation may clamp demand by cash, max position, or passive_only flag.
 */
double agent_compute_demand(const Agent *a,
                            double market_price,
                            double global_shock,
                            double avg_neighbor_belief);

/*
 * Apply an execution result to the agent state.
 *  - executed_quantity: signed integer (positive buy, negative sell)
 *  - execution_price: price at which trade executed
 *
 * Updates:
 *  - a->position and a->cash adjusted
 *  - may update internal learning variables if desired
 */
void agent_apply_execution(Agent *a, int executed_quantity, double execution_price);

/*
 * Update agent belief using market information and optional shock.
 * This separates belief dynamics from compute_demand so experiments can vary learning rules.
 *
 * Typical update:
 *  a->belief += belief_update_rate * (observed_signal - a->belief) + shock_component
 */
void agent_update_belief(Agent *a, double observed_price, double global_shock, double avg_market_signal);

/*
 * Apply a news/information shock directly to agent parameters or belief.
 * Useful to model heterogeneous reaction to news (e.g., institutions less reactive).
 */
void agent_apply_shock(Agent *a, double shock_strength);

/*
 * Utility: deallocate any dynamically allocated memory within agent (e.g., neighbors array).
 * If you allocate neighbors outside and assign pointer, do not free here.
 */
void agent_free(Agent *a);

/*
 * Utility: return a newly-allocated JSON string describing the agent state.
 * Caller must free() the returned string.
 */
char *agent_to_json(const Agent *a);

/* -------------------- Inline helpers (optional, small utilities) -------------------- */

/*
 * Position penalty function (example):
 *  - prevents unlimited position growth; can be tuned or replaced by risk limits
 */
static inline double position_penalty(int position) {
    /* simple smooth penalty: p / (1 + |p|) to keep units comparable */
    double p = (double)position;
    return p / (1.0 + fabs(p));
}

#endif /* JUMPSIM_AGENT_H */
