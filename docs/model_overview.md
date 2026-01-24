# JumpSim — Model Overview
**Endogenous Price Jumps from Agent Interaction and Information Flow**

---

## 1. Objective

JumpSim is an agent-based financial market simulator designed to study how abrupt price movements (jumps) can emerge endogenously from heterogeneous trader behavior and information-driven dynamics, without imposing any exogenous jump process.

The primary research question is:

> Can market microstructure and behavioral interaction alone generate discontinuous price dynamics under finite liquidity?

This framework enables controlled experiments on market stability, herding, information cascades, and liquidity fragility.

---

## 2. Modeling Philosophy

Traditional financial models (e.g., Geometric Brownian Motion, Merton Jump Diffusion) treat price jumps as externally injected stochastic events. JumpSim instead adopts a bottom-up approach:

- Agents interact locally and respond to imperfect information.
- Order flow aggregates into excess demand.
- Prices adjust via a liquidity-constrained impact mechanism.
- Large price moves arise naturally when coordination or shocks amplify demand imbalances.

No artificial randomness is added directly to the price process.

---

## 3. Agent Types

The market contains heterogeneous agents representing stylized trading archetypes:

### Retail Traders
- Momentum-driven and behaviorally biased.
- Strong sensitivity to peer behavior (herding).
- Faster belief adaptation.
- Amplify volatility during coordinated reactions.

### Institutional Traders
- Fundamental-value oriented.
- Higher risk aversion and inventory discipline.
- Slower belief updates.
- Provide stabilizing liquidity.

### Noise Traders
- Randomized trading behavior.
- Provide background liquidity and stochasticity.
- No informational advantage.

Each agent maintains:
- A subjective belief about asset value.
- A position (inventory).
- Behavioral parameters controlling aggressiveness, risk tolerance, and social influence.

---

## 4. Information Process

Exogenous information arrives as rare news shocks with heavy-tailed magnitudes and regime switching (calm vs stress).  

Information does not affect prices directly. Instead:

1. Global news generates a macro signal.
2. Information diffuses through the agent network with:
   - Limited attention,
   - Temporal decay,
   - Social amplification.
3. Each agent updates its belief based on received information.

This structure reflects realistic information asymmetry and delayed market response.

---

## 5. Agent Decision Model

At each time step, agents compute desired demand as a function of:

- **Valuation signal:** Difference between belief and current market price.
- **Inventory risk:** Penalty for holding large positions.
- **Herding influence:** Deviation from neighbor beliefs.
- **Idiosyncratic noise:** Behavioral randomness.
- **Information shock impact.**

Formally:
Demand ≈ α (belief − price)
− β (inventory penalty)
+ γ (neighbor influence)
+ ε (noise)
+ shock


This formulation is consistent with agent-based market literature and bounded rationality models.

---

## 6. Price Formation

The market aggregates all agent demands into excess demand.

Price adjustment follows a linear impact rule:

ΔP = κ × (ExcessDemand / Liquidity)


Where:
- κ is the impact coefficient,
- Liquidity controls market depth.

This mechanism reflects microstructure-based price impact rather than stochastic diffusion.

Circuit breakers limit extreme single-step price movements for stability analysis.

---

## 7. Emergence of Price Jumps

Price jumps emerge when:

- Many agents synchronize beliefs (herding),
- News shocks propagate rapidly,
- Liquidity is insufficient to absorb order flow,
- Excess demand spikes suddenly.

Jumps are therefore emergent phenomena rather than model assumptions.

---

## 8. Statistical Validation

The simulator computes online statistics:

- Log returns
- Volatility (EWMA)
- Jump frequency (threshold-based)
- Kurtosis (fat tails)

These metrics allow validation against empirical stylized facts of financial markets:
- Heavy-tailed returns,
- Volatility clustering,
- Intermittent extreme events.

---

## 9. Experimental Design

Experiments are controlled using JSON configuration files:

- `baseline_config.json` — stable reference regime.
- `high_herding_config.json` — behavioral amplification stress test.
- `low_liquidity_config.json` — market fragility stress test.

All experiments are reproducible via explicit random seeds.

---

## 10. Limitations and Extensions

Current limitations:
- No explicit limit order book.
- Simplified network topology.
- No leverage or margin dynamics.

Planned extensions:
- Order book microstructure,
- Endogenous liquidity provision,
- Adaptive learning rules,
- Multi-asset coupling,
- Parallel Monte Carlo experiments.

---

## 11. Intended Use

JumpSim is intended as:
- A research sandbox for market dynamics,
- A teaching tool for computational economics,
- A foundation for further academic experimentation.


