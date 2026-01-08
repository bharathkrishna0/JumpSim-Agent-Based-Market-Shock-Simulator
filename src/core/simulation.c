#include "agent.h"
#include "market.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* ---------------- Simulation Parameters ---------------- */

#define NUM_AGENTS      400
#define TIME_STEPS      3000

#define RETAIL_SHARE        0.6
#define INSTITUTION_SHARE   0.3
#define NOISE_SHARE         0.1

#define INITIAL_PRICE   100.0
#define INITIAL_LIQUIDITY 1200.0

/* ---------------- Utility Random ---------------- */

static double uniform_random() {
    return rand() / (double)RAND_MAX;
}

/* ---------------- News Shock Process ---------------- */

/*
   Rare-event shock process.

   Economic meaning:
   - Most periods: no important news
   - Occasionally: large information shock
   - Shock affects many agents simultaneously
*/

static double generate_news_shock() {
    double p = uniform_random();

    if (p < 0.015) {                     /* 1.5% probability */
        double magnitude = (uniform_random() - 0.5) * 12.0;
        return magnitude;
    }

    return 0.0;
}

/* ---------------- Agent Initialization ---------------- */

static void initialize_agents(Agent agents[]) {

    for (int i = 0; i < NUM_AGENTS; i++) {

        double r = uniform_random();
        AgentType type;

        if (r < RETAIL_SHARE) type = AGENT_RETAIL;
        else if (r < RETAIL_SHARE + INSTITUTION_SHARE) type = AGENT_INSTITUTION;
        else type = AGENT_NOISE;

        double aggressiveness;
        double risk_aversion;
        double network_influence;
        double noise_std;

        if (type == AGENT_RETAIL) {
            aggressiveness = 1.0;
            risk_aversion = 0.2;
            network_influence = 0.7;
            noise_std = 0.6;
        }
        else if (type == AGENT_INSTITUTION) {
            aggressiveness = 0.5;
            risk_aversion = 0.8;
            network_influence = 0.1;
            noise_std = 0.2;
        }
        else {
            aggressiveness = 0.2;
            risk_aversion = 0.1;
            network_influence = 0.0;
            noise_std = 1.0;
        }

        char name[32];
        snprintf(name, sizeof(name), "Agent_%d", i);

        agent_init(&agents[i],
                   (AgentId)i,
                   type,
                   name,
                   INITIAL_PRICE,
                   aggressiveness,
                   1.0,         /* trade size scale */
                   risk_aversion,
                   0.02,        /* liquidity tolerance */
                   0.05,        /* belief update rate */
                   network_influence,
                   noise_std,
                   INITIAL_PRICE, /* fundamental anchor */
                   rand());
    }
}

/* ---------------- Main Simulation ---------------- */

int main() {

    srand((unsigned)time(NULL));

    Agent agents[NUM_AGENTS];
    Market market;

    /* Initialize system */
    initialize_agents(agents);

    market_init(&market,
                INITIAL_PRICE,
                INITIAL_LIQUIDITY,
                1.0,        /* impact coefficient */
                0.94,       /* volatility decay */
                5.0);       /* max price change */

    FILE *fp = fopen("prices.csv", "w");
    fprintf(fp, "time,price,log_return,volatility,shock\n");

    /* ---------------- Time Loop ---------------- */

    for (int t = 0; t < TIME_STEPS; t++) {

        market_begin_step(&market);

        /* Generate global information shock */
        double shock = generate_news_shock();

        /* Optional: broadcast shock to agents */
        if (shock != 0.0) {
            for (int i = 0; i < NUM_AGENTS; i++) {
                agent_apply_shock(&agents[i], shock);
            }
        }

        /* Compute average belief (simple proxy for sentiment) */
        double avg_belief = 0.0;
        for (int i = 0; i < NUM_AGENTS; i++) {
            avg_belief += agents[i].belief;
        }
        avg_belief /= NUM_AGENTS;

        /* Collect agent demands */
        for (int i = 0; i < NUM_AGENTS; i++) {

            double demand =
                agent_compute_demand(&agents[i],
                                      market.price,
                                      shock,
                                      avg_belief);

            market_add_demand(&market, demand);

            /* Apply execution immediately (mean-field assumption) */
            int executed = (int)round(demand);
            agent_apply_execution(&agents[i], executed, market.price);
        }

        /* Clear market and update price */
        market_clear(&market);
        market_update_volatility(&market);

        /* Update agent beliefs after observing price */
        for (int i = 0; i < NUM_AGENTS; i++) {
            agent_update_belief(&agents[i],
                                market.price,
                                shock,
                                avg_belief);
        }

        /* Logging */
        double logret = market_log_return(&market);

        fprintf(fp, "%d,%f,%f,%f,%f\n",
                t,
                market.price,
                logret,
                market.volatility,
                shock);

        /* Optional: simple circuit breaker */
        if (fabs(logret) > 0.15) {
            market_halt(&market);
        }
        else {
            market_resume(&market);
        }
    }

    fclose(fp);

    printf("Simulation completed. Output saved to prices.csv\n");
    return 0;
}
