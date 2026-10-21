"""Plot a JumpSim run from prices.csv (written by the C simulator).

Usage:
    python scripts/plot_results.py [prices.csv] [--out jumpsim_run.png] [--jump-threshold 0.02]
"""

import argparse

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("csv", nargs="?", default="prices.csv")
    parser.add_argument("--out", default="jumpsim_run.png")
    parser.add_argument("--jump-threshold", type=float, default=0.02,
                        help="|log return| above this is marked as a jump")
    args = parser.parse_args()

    df = pd.read_csv(args.csv)
    shocks = df[df["shock"] != 0.0]
    jumps = df[df["log_return"].abs() > args.jump_threshold]
    r = df["log_return"]
    kurtosis = ((r - r.mean()) ** 4).mean() / r.var(ddof=0) ** 2 - 3.0

    print(f"steps:            {len(df)}")
    print(f"price start/end:  {df['price'].iloc[0]:.2f} to {df['price'].iloc[-1]:.2f}")
    print(f"price min/max:    {df['price'].min():.2f} / {df['price'].max():.2f}")
    print(f"news shocks:      {len(shocks)}")
    print(f"jumps > {args.jump_threshold}:     {len(jumps)}")
    print(f"max abs return:   {r.abs().max():.4f}")
    print(f"excess kurtosis:  {kurtosis:.2f}")

    fig, (ax_p, ax_r, ax_v) = plt.subplots(3, 1, figsize=(11, 8), sharex=True)

    ax_p.plot(df["time"], df["price"], lw=0.9, color="tab:blue", label="price")
    ax_p.scatter(shocks["time"], shocks["price"], s=18, color="tab:red", zorder=3,
                 label="news shock")
    ax_p.set_ylabel("price")
    ax_p.legend(loc="best")
    ax_p.set_title("JumpSim run: price path, returns and EWMA volatility")

    ax_r.plot(df["time"], r, lw=0.6, color="tab:gray")
    ax_r.scatter(jumps["time"], jumps["log_return"], s=14, color="tab:orange", zorder=3,
                 label=f"|log return| > {args.jump_threshold}")
    ax_r.set_ylabel("log return")
    ax_r.legend(loc="best")

    ax_v.plot(df["time"], np.sqrt(df["volatility"]), lw=0.9, color="tab:purple")
    ax_v.set_ylabel("EWMA volatility")
    ax_v.set_xlabel("time step")

    fig.tight_layout()
    fig.savefig(args.out, dpi=120)
    print(f"saved {args.out}")


if __name__ == "__main__":
    main()
