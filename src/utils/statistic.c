#ifndef JUMPSIM_STATISTICS_H
#define JUMPSIM_STATISTICS_H

#include <stddef.h>

/*
 * statistics.h
 * -------------
 * Online statistical estimators for JumpSim.
 *
 * Supports:
 *  - Mean and variance (Welford)
 *  - Kurtosis (fat-tail detection)
 *  - Jump detection
 *  - Volatility clustering proxy
 */

typedef struct {
    /* Streaming moments */
    long n;
    double mean;
    double m2;
    double m3;
    double m4;

    /* Jump statistics */
    long jump_count;
    double jump_threshold;

    /* Volatility clustering proxy */
    double abs_return_ewma;
    double ewma_decay;

} Stats;

/* Initialize statistics */
void stats_init(Stats *s, double jump_threshold, double ewma_decay);

/* Update with new log return */
void stats_update(Stats *s, double log_return);

/* Accessors */
double stats_variance(const Stats *s);
double stats_kurtosis(const Stats *s);

/* Jump metrics */
int stats_is_jump(const Stats *s, double log_return);
double stats_jump_frequency(const Stats *s);

#endif /* JUMPSIM_STATISTICS_H */
