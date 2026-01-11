#ifndef JUMPSIM_RNG_H
#define JUMPSIM_RNG_H

#include <stdint.h>

/*
 * rng.h
 * -----
 * Central random number generator for JumpSim.
 *
 * Design goals:
 *  - Reproducible
 *  - Fast
 *  - Platform-independent
 *  - Suitable for Monte Carlo simulation
 *
 * Provides:
 *  - Uniform(0,1)
 *  - Normal(0,1)
 */

void rng_seed(uint64_t seed);

/* Uniform random number in (0,1) */
double rng_uniform();

/* Standard normal random number (mean 0, std 1) */
double rng_normal();

#endif /* JUMPSIM_RNG_H */
