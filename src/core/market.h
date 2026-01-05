#ifndef JUMPSIM_MARKET_H
#define JUMPSIM_MARKET_H

/*
 * market.h
 * --------
 * Market-level state and price formation interface for JumpSim.
 *
 * Economic interpretation:
 *  - The market aggregates excess demand from agents
 *  - Prices adjust under finite liquidity
 *  - No exogenous stochastic price process is imposed
 *
 * This file defines:
 *  - Market state variables
 *  - Price impact mechanism (interface only)
 *  - Volatility and diagnostic tracking
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* -------------------- Market Structure -------------------- */

typedef struct Market {
    /* Core price state */
    double price;              /* current market price */
    double last_price;         /* previous price (for returns) */

    /* Liquidity & impact */
    double liquidity;          /* depth of market (higher = more stable) */
    double impact_coefficient; /* scales excess demand into price change */

    /* Volatility diagnostics */
    double volatility;         /* rolling volatility proxy */
    double volatility_decay;   /* EWMA decay parameter */

    /* Order flow tracking */
    double cumulative_demand;  /* sum of signed demand in current step */
    double cumulative_volume;  /* absolute traded volume */

    /* Time bookkeeping */
    uint64_t time;             /* discrete time index */

    /* Stability / safety controls */
    double max_price_change;   /* cap on single-step price move */
    bool trading_halted;       /* circuit breaker flag */
} Market;

/* -------------------- API (implemented in market.c) -------------------- */

/*
 * Initialize the market.
 *
 * Parameters:
 *  - init_price: starting price level
 *  - liquidity: market depth (inverse of price impact)
 *  - impact_coefficient: maps excess demand to price movement
 *  - volatility_decay: EWMA decay for volatility estimation (e.g., 0.94)
 *  - max_price_change: hard cap on |Δprice| per step
 */
void market_init(Market *m,
                 double init_price,
                 double liquidity,
                 double impact_coefficient,
                 double volatility_decay,
                 double max_price_change);

/*
 * Reset per-step aggregates before collecting agent demand.
 * Must be called at the start of each simulation step.
 */
void market_begin_step(Market *m);

/*
 * Submit agent demand to the market.
 *
 * Arguments:
 *  - signed_demand: positive = buy pressure, negative = sell pressure
 *
 * This does NOT update price yet; it only accumulates order flow.
 */
void market_add_demand(Market *m, double signed_demand);

/*
 * Clear the market and update price.
 *
 * Economic logic (conceptual):
 *  - excess_demand = cumulative_demand
 *  - price_change ∝ excess_demand / liquidity
 *  - apply impact, caps, and update volatility
 *
 * Returns:
 *  - new market price
 */
double market_clear(Market *m);

/*
 * Compute log-return from last step.
 * Useful for volatility and jump detection.
 */
double market_log_return(const Market *m);

/*
 * Update market volatility estimate.
 * Typically called after market_clear().
 */
void market_update_volatility(Market *m);

/*
 * Trigger a trading halt (circuit breaker).
 * Price will not update while halted.
 */
void market_halt(Market *m);

/*
 * Resume trading after halt.
 */
void market_resume(Market *m);

#endif /* JUMPSIM_MARKET_H */
