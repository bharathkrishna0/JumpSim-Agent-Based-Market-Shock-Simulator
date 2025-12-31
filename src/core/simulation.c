#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_AGENTS 500
#define TIME_STEPS 1000

/* ---------------- Agent Types ---------------- */
typedef enum {
    RETAIL,
    INSTITUTION,
    NOISE
} AgentType;

/* ---------------- Agent Structure ---------------- */
typedef struct {
    AgentType type;
    double belief;      // expected price
    double aggressiveness;
    int position;
} Agent;


/* ---------------- Market Structure ---------------- */
typedef struct {
    double price;
    double liquidity;
} Market;

/* ---------------- Random Helpers ---------------- */
double uniform_random() {
    return rand() / (double)RAND_MAX;
}

double normal_random() {
    // Box-Muller transform
    double u1 = uniform_random();
    double u2 = uniform_random();
    return sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
}

/* ---------------- Initialize Agents ---------------- */
void init_agents(Agent agents[], double init_price) {
    for (int i = 0; i < NUM_AGENTS; i++) {
        double r = uniform_random();

        if (r < 0.6) agents[i].type = RETAIL;
        else if (r < 0.9) agents[i].type = INSTITUTION;
        else agents[i].type = NOISE;

        agents[i].belief = init_price;
        agents[i].position = 0;

        if (agents[i].type == RETAIL)
            agents[i].aggressiveness = 0.8;
        else if (agents[i].type == INSTITUTION)
            agents[i].aggressiveness = 0.3;
        else
            agents[i].aggressiveness = 0.1;
    }
}


/* ---------------- News Shock ---------------- */
double news_shock(int t) {
    // Rare but strong
    if (uniform_random() < 0.02) {
        return normal_random() * 5.0;
    }
    return 0.0;
}
/* ---------------- Agent Decision ---------------- */
double agent_demand(Agent *a, double price, double shock) {
    double demand = 0.0;

    if (a->type == RETAIL) {
        // momentum + herding
        a->belief += 0.5 * shock;
        demand = a->aggressiveness * (a->belief - price);
    }
    else if (a->type == INSTITUTION) {
        // mean-reversion
        demand = a->aggressiveness * (a->belief - price);
    }
    else {
        // noise trader
        demand = normal_random();
    }

    return demand;
}

/* ---------------- Main Simulation ---------------- */
int main() {
    srand(time(NULL));

    Agent agents[NUM_AGENTS];
    Market market;

    market.price = 100.0;
    market.liquidity = 1000.0;

    init_agents(agents, market.price);

    FILE *fp = fopen("prices.csv", "w");
    fprintf(fp, "time,price\n");

    for (int t = 0; t < TIME_STEPS; t++) {
        double total_demand = 0.0;
        double shock = news_shock(t);


        for (int i = 0; i < NUM_AGENTS; i++) {
            total_demand += agent_demand(&agents[i], market.price, shock);
        }

        // Price formation rule
        double price_change = total_demand / market.liquidity;
        market.price += price_change;

        fprintf(fp, "%d,%f\n", t, market.price);

        // Update beliefs slowly
        for (int i = 0; i < NUM_AGENTS; i++) {
            agents[i].belief += 0.01 * (market.price - agents[i].belief);
        }
    }
    fclose(fp);
    return 0;
}
