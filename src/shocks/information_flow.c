#include "agent.h"
#include <stdlib.h>
#include <math.h>

/*
 * information_flow.c
 * -------------------
 * Models how exogenous news propagates through the agent population.
 *
 * Economic principles:
 *  1. Information does NOT reach all agents instantly.
 *  2. Agents filter information differently (attention, trust, latency).
 *  3. Social networks amplify or dampen signals (herding, cascades).
 *  4. Institutions react slower but more accurately.
 *
 * This module transforms a global shock into heterogeneous local signals.
 */

/* ---------------- Configuration ---------------- */

#define MAX_PROPAGATION_STEPS 3
#define BASE_ATTENTION 0.6

/* ---------------- Internal Helpers ---------------- */

/*
 * Attention weight models limited attention / media filtering.
 * Retail agents overweight salient news.
 * Institutions dampen noisy signals.
 */
static double attention_weight(AgentType type) {
    if (type == AGENT_RETAIL)        return 1.2;
    if (type == AGENT_INSTITUTION)  return 0.6;
    return 0.9;  /* noise traders */
}

/*
 * Delay filter simulates reaction latency.
 * Not all information is acted upon immediately.
 */
static double temporal_decay(int step) {
    /* exponential decay */
    return exp(-0.8 * step);
}

/* ---------------- Core Diffusion Logic ---------------- */

/*
 * Propagate a global news shock through the agent network.
 *
 * Parameters:
 *  - agents: array of agents
 *  - n_agents: number of agents
 *  - global_shock: macro news signal
 *
 * Effect:
 *  - Each agent receives a filtered version of the shock.
 *  - Neighbor beliefs influence secondary propagation.
 *  - No direct price manipulation.
 */
void information_propagate(Agent *agents,
                           size_t n_agents,
                           double global_shock)
{
    if (fabs(global_shock) < 1e-9) return;

    /* Temporary buffers */
    double *local_signal = calloc(n_agents, sizeof(double));
    double *next_signal  = calloc(n_agents, sizeof(double));

    /* ---------------- Step 0: Direct exposure ---------------- */

    for (size_t i = 0; i < n_agents; i++) {
        double w = attention_weight(agents[i].type);
        local_signal[i] = BASE_ATTENTION * w * global_shock;
    }

    /* ---------------- Network propagation ---------------- */

    for (int step = 1; step <= MAX_PROPAGATION_STEPS; step++) {

        double decay = temporal_decay(step);

        for (size_t i = 0; i < n_agents; i++) {

            if (agents[i].neighbor_count == 0) continue;

            double neighbor_avg = 0.0;

            for (size_t k = 0; k < agents[i].neighbor_count; k++) {
                int nid = agents[i].neighbors[k];
                neighbor_avg += local_signal[nid];
            }

            neighbor_avg /= agents[i].neighbor_count;

            /* Secondary signal from social transmission */
            next_signal[i] += decay * agents[i].network_influence * neighbor_avg;
        }

        /* Accumulate and reset */
        for (size_t i = 0; i < n_agents; i++) {
            local_signal[i] += next_signal[i];
            next_signal[i] = 0.0;
        }
    }

    /* ---------------- Apply to agent beliefs ---------------- */

    for (size_t i = 0; i < n_agents; i++) {

        /*
         Belief update from information signal:
           belief += signal
         */

        agents[i].belief += local_signal[i];
    }

    free(local_signal);
    free(next_signal);
}
