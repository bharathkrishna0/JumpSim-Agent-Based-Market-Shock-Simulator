# 📈 JumpSim: Agent-Based Market Shock Simulator

JumpSim is a simulation framework that models asset price dynamics through agent-based interactions and information-driven market shocks. Unlike traditional stochastic models, JumpSim allows price jumps to **emerge endogenously** from trader behavior, herding, and sudden external news events.

## 🧠 Project Motivation

Markets are not just numbers—they’re made up of humans and algorithms reacting to information. This simulator captures how price jumps can arise **organically**, without hardcoding them, through:
- Retail momentum chasing
- Institutional value-driven decisions
- Random noise trading
- Sudden global events or news shocks

By modeling the **psychology of trading**, JumpSim explores how **systemic risks and flash crashes** might unfold.

---

## ⚙️ Features

- 👥 Agent types: `Retail`, `Institutional`, and `Noise` traders
- 📊 Simulated price evolution based on demand-supply imbalance
- 📉 Jump events triggered by **Poisson-distributed news shocks**
- 🔁 Herding effect: synchronized agent behavior post-news
- 📈 Realistic price path with volatility clustering and sudden drops/spikes
- 📊 Graphs: Price path, jump points, volatility trends

---

## 🚀 How It Works

1. **Initialize Agents**: Each with distinct behavior models
2. **Simulate Time Steps**: Each agent decides to buy/sell/hold
3. **External Shocks**: Simulated using a Poisson process
4. **Market Reaction**: Agents react, sometimes in sync → causing price jumps
5. **Visualization**: Plot asset price evolution, mark jumps, analyze volatility

---

## 📦 Tech Stack

- `Python`
- `NumPy`, `Pandas` – Numerical operations
- `Matplotlib`, `Seaborn` – Visualization
- *(Optional)* `Mesa` – For advanced agent-based simulation

---


## 🛠️ Installation

```bash
git clone https://github.com/bharathkrishna0/jumpsim.git
cd jumpsim
pip install -r requirements.txt
python main.py
