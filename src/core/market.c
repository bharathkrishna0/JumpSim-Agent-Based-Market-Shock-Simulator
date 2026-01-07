#include "market.h"
#include <math.h>
#include <stdio.h>

/* ----------------------------------------------------
   Market Initialization
---------------------------------------------------- */

void market_init(Market *m,
                 double init_price,
                 double liquidity,
                 double impact_coefficient,
                 double volatility_decay,
                 double max_price_change)
{
    m->price = init_price;
    m->last_price = init_price;

    m->liquidity = liquidity;
    m->impact_coefficient = impact_coefficient;

    m->volatility = 0.0;
    m->volatility_decay = volatility_decay;

    m->cumulative_demand = 0.0;
    m->cumulative_volume = 0.0;

    m->time = 0;

    m->max_price_change = max_price_change;
    m->trading_halted = false;
}

/* ----------------------------------------------------
   Begin Simulation Step
---------------------------------------------------- */

void market_begin_step(Market *m)
{
    /*
     Reset per-step order flow accumulators.

     Economic meaning:
     - Each time step represents a clearing window.
     - Agents submit demand within this window.
     - Market aggregates before clearing.
    */

    m->cumulative_demand = 0.0;
    m->cumulative_volume = 0.0;
}

/* ----------------------------------------------------
   Collect Agent Demand
---------------------------------------------------- */

void market_add_demand(Market *m, double signed_demand)
{
    /*
     signed_demand:
       +ve => net buying pressure
       -ve => net selling pressure

     We track:
       - cumulative_demand : directional imbalance
       - cumulative_volume : liquidity usage proxy
    */

    m->cumulative_demand += signed_demand;
    m->cumulative_volume += fabs(signed_demand);
}

/* ----------------------------------------------------
   Market Clearing & Price Formation
---------------------------------------------------- */

double market_clear(Market *m)
{
    /*
     CORE ECONOMIC MODEL

     Price impact model:
       ΔP = κ * (ExcessDemand / Liquidity)

     where:
       κ  = impact_coefficient
       Liquidity = market depth
       ExcessDemand = sum of all agent demands

     This is a linear impact approximation used in
     market microstructure literature.

     IMPORTANT:
     - There is NO random price term here.
     - All volatility and jumps come from agent behavior.
    */

    if (m->trading_halted) {
        return m->price;
    }

    m->last_price = m->price;

    double excess_demand = m->cumulative_demand;

    /* Normalize by liquidity */
    double normalized_flow = excess_demand / m->liquidity;

    /* Price impact */
    double raw_price_change =
        m->impact_coefficient * normalized_flow;

    /* Safety: cap extreme single-step moves */
    if (raw_price_change > m->max_price_change)
        raw_price_change = m->max_price_change;
    else if (raw_price_change < -m->max_price_change)
        raw_price_change = -m->max_price_change;

    m->price += raw_price_change;

    /* Prevent negative or zero prices */
    if (m->price < 1e-6) {
        m->price = 1e-6;
    }

    m->time++;

    return m->price;
}

/* ----------------------------------------------------
   Log Return Computation
---------------------------------------------------- */

double market_log_return(const Market *m)
{
    /*
     Log-return:
       r_t = ln(P_t / P_{t-1})

     Used for:
       - volatility estimation
       - jump detection
       - stylized fact analysis
    */

    if (m->last_price <= 0.0 || m->price <= 0.0) {
        return 0.0;
    }

    return log(m->price / m->last_price);
}

/* ----------------------------------------------------
   Volatility Update (EWMA)
---------------------------------------------------- */

void market_update_volatility(Market *m)
{
    /*
     Exponentially Weighted Moving Average (EWMA):

       σ²_t = λ σ²_{t-1} + (1 − λ) r_t²

     This is the same structure used in RiskMetrics.
    */

    double r = market_log_return(m);
    double r2 = r * r;

    m->volatility =
        m->volatility_decay * m->volatility
        + (1.0 - m->volatility_decay) * r2;
}

/* ----------------------------------------------------
   Circuit Breakers
---------------------------------------------------- */

void market_halt(Market *m)
{
    /*
     Trading halt:
     - Price no longer updates
     - Order flow may still be measured
     */

    m->trading_halted = true;
}

void market_resume(Market *m)
{
    m->trading_halted = false;
}
