import math
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from solvers import bisection, fixed_point, newton

# Pretty table printer
def print_table(rows, header):
    col_w = [max(len(str(r[i])) for r in [header] + rows) for i in range(len(header))]
    fmt = " | ".join(f"{{:<{w}}}" for w in col_w)
    sep = "-+-".join("-" * w for w in col_w)
    print(fmt.format(*header))
    print(sep)
    for r in rows:
        print(fmt.format(*r))
    print()


# Problem 1 :  f(x) = x³ − x − 2
f1      = lambda x: x**3 - x - 2
df1     = lambda x: 3*x**2 - 1

# True root (numerically)
x_star1 = 1.5213797068045676

# Bisection [1, 2]
root_b1, res_b1, it_b1, hist_b1 = bisection(f1, 1.0, 2.0)

# Fixed-Point g1(x) = (x + 2)^(1/3)   (convergent)
g1_good = lambda x: (x + 2) ** (1.0 / 3.0)
root_fp1, res_fp1, it_fp1, hist_fp1 = fixed_point(g1_good, 1.5, f=f1)

# Fixed-Point g2(x) = x^3 - 2   (divergent)
g1_bad  = lambda x: x**3 - 2
root_fp1b, res_fp1b, it_fp1b, hist_fp1b = fixed_point(g1_bad, 1.5, f=f1)

# Newton
root_n1, res_n1, it_n1, hist_n1 = newton(f1, df1, 1.5)

print("=" * 70)
print("Problem 1: f(x) = x³ − x − 2")
print("=" * 70)
rows1 = [
    ("Bisection",          "[1, 2]",                str(it_b1),   f"{root_b1:.10f}",  f"{res_b1:.2e}"),
    ("Fixed-Point (good)", "g=(x+2)^(1/3), x0=1.5", str(it_fp1),  f"{root_fp1:.10f}", f"{res_fp1:.2e}"),
    ("Fixed-Point (bad)",  "g=x³−2, x0=1.5",        str(it_fp1b), f"{root_fp1b:.6g}", f"{res_fp1b:.2e}"),
    ("Newton",             "x0=1.5",                 str(it_n1),   f"{root_n1:.10f}",  f"{res_n1:.2e}"),
]
print_table(rows1, ("Method", "Init", "Iters", "Root", "|f(root)|"))


# Problem 2 :  f(x) = cos(x) − x
f2      = lambda x: math.cos(x) - x
df2     = lambda x: -math.sin(x) - 1

x_star2 = 0.7390851332151607 # true root of cos(x)-x=0 (used for error plots)

# Newton with several initial guesses
newton2_results = {}
for x0 in [0.0, 1.0, 5.0, 10.0]:
    root, res, it, hist = newton(f2, df2, x0)
    newton2_results[x0] = (root, res, it, hist)

# Bisection for reference
root_b2, res_b2, it_b2, hist_b2 = bisection(f2, 0.0, 1.0)

# Fixed-Point  g(x) = cos(x)
g2 = lambda x: math.cos(x)
root_fp2, res_fp2, it_fp2, hist_fp2 = fixed_point(g2, 0.5, f=f2)

print("=" * 70)
print("Problem 2: f(x) = cos(x) − x")
print("=" * 70)
rows2 = [
    ("Bisection", "[0, 1]", str(it_b2), f"{root_b2:.10f}", f"{res_b2:.2e}"),
    ("Fixed-Point", "g=cos(x), x0=0.5", str(it_fp2), f"{root_fp2:.10f}", f"{res_fp2:.2e}"),
]
for x0, (root, res, it, hist) in newton2_results.items():
    rows2.append(("Newton", f"x0={x0}", str(it), f"{root:.10f}", f"{res:.2e}"))
print_table(rows2, ("Method", "Init", "Iters", "Root", "|f(root)|"))


# Problem 3 :  f(x) = x³   (triple root at 0)
f3      = lambda x: x**3
df3     = lambda x: 3*x**2
x_star3 = 0.0

# Bisection [-1, 1] for example
root_b3, res_b3, it_b3, hist_b3 = bisection(f3, -0.5, 1.0)

# Fixed-Point g(x) = -x^3 + x   (rearranged from x^3 = 0 => x = -x^3 + x)
# |g'(x*)| = |-3x^2 + 1| = 1 at x*=0, so convergence is marginal/slow
g3_fp = lambda x: -x**3 + x
root_fp3, res_fp3, it_fp3, hist_fp3 = fixed_point(g3_fp, 0.5, f=f3)

# Newton
root_n3, res_n3, it_n3, hist_n3 = newton(f3, df3, 1.0)

print("=" * 70)
print("Problem 3: f(x) = x^3  (multiple root at x = 0)")
print("=" * 70)
rows3 = [
    ("Bisection",    "[-0.5, 1]",                 str(it_b3),  f"{root_b3:.10f}",  f"{res_b3:.2e}"),
    ("Fixed-Point",  "g=-x^3+x, x0=0.5",          str(it_fp3), f"{root_fp3:.10f}", f"{res_fp3:.2e}"),
    ("Newton",       "x0=1.0",                     str(it_n3),  f"{root_n3:.10f}",  f"{res_n3:.2e}"),
]
print_table(rows3, ("Method", "Init", "Iters", "Root", "|f(root)|"))

# Convergence ratio analysis for Problem 3
print("Newton convergence ratios for f(x)=x^3 :")
errors3 = [abs(x - x_star3) for x in hist_n3]
for k in range(2, min(len(errors3), 25)):
    if errors3[k-1] > 1e-15:
        ratio = errors3[k] / errors3[k-1]
        print(f"  k={k:3d}  |e_k|={errors3[k]:.6e}  ratio |e_k|/|e_{{k-1}}|={ratio:.6f}")
print()


# Convergence Plots  (semilogy)
def make_convergence_plot(series, title, filename, xlabel="Iteration", ylabel="Error |x_k − x*|"):
    fig, ax = plt.subplots(figsize=(8, 5))
    for label, (hist, xs) in series.items():
        errs = [abs(x - xs) for x in hist]
        errs = [e if e > 0 else 1e-17 for e in errs]
        ax.semilogy(range(len(errs)), errs, 'o-', markersize=4, label=label)
    ax.set_xlabel(xlabel, fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_title(title, fontsize=13)
    ax.legend(fontsize=10)
    ax.grid(True, which="both", ls="--", alpha=0.5)
    fig.tight_layout()
    fig.savefig(filename, dpi=150)
    plt.close(fig)
    print(f"Saved plot → {filename}")


# Plot 1 : Problem 1
make_convergence_plot(
    {
        "Bisection [1,2]":              (hist_b1,  x_star1),
        "Fixed-Pt  g=(x+2)^{1/3}":     (hist_fp1, x_star1),
        "Newton  x₀=1.5":              (hist_n1,  x_star1),
    },
    "Problem 1: f(x) = x³ − x − 2  —  Convergence",
    "plot_problem1.png",
)

# Plot 2 : Problem 2
series2 = {
    "Bisection [0,1]": (hist_b2, x_star2),
    "Fixed-Pt  g=cos(x)": (hist_fp2, x_star2),
}
for x0, (root, res, it, hist) in newton2_results.items():
    series2[f"Newton x₀={x0}"] = (hist, x_star2)

make_convergence_plot(
    series2,
    "Problem 2: f(x) = cos(x) − x  —  Convergence",
    "plot_problem2.png",
)

# Plot 3 : Problem 3
make_convergence_plot(
    {
        "Bisection [-0.5,1]":         (hist_b3,  x_star3),
        "Fixed-Pt  g=-x^3+x":        (hist_fp3, x_star3),
        "Newton  x0=1.0":             (hist_n3,  x_star3),
    },
    "Problem 3: f(x) = x^3  (triple root)  --  Convergence",
    "plot_problem3.png",
)

# Plot 4 : Fixed-Point divergence demo (Problem 1, bad g)
fig, ax = plt.subplots(figsize=(8, 5))
errs_bad = [abs(x - x_star1) for x in hist_fp1b[:30]]
errs_bad = [e if e > 0 else 1e-17 for e in errs_bad]
ax.semilogy(range(len(errs_bad)), errs_bad, 's-', color='red', markersize=4,
            label="Fixed-Pt  g=x³−2 (divergent)")
ax.set_xlabel("Iteration", fontsize=12)
ax.set_ylabel("Error |x_k − x*|", fontsize=12)
ax.set_title("Problem 1: Divergent Fixed-Point  g(x)=x³−2", fontsize=13)
ax.legend(fontsize=10)
ax.grid(True, which="both", ls="--", alpha=0.5)
fig.tight_layout()
fig.savefig("plot_fp_divergent.png", dpi=150)
plt.close(fig)
print("Saved plot → plot_fp_divergent.png")

# Plot 5 : Convergence rate comparison for Problem 3
fig, ax = plt.subplots(figsize=(8, 5))
# Newton on x^3 converges linearly with ratio ≈ 2/3
ratios3 = []
for k in range(1, len(errors3)):
    if errors3[k-1] > 1e-15:
        ratios3.append(errors3[k] / errors3[k-1])
ax.plot(range(1, len(ratios3)+1), ratios3, 'o-', color='purple', markersize=5)
ax.axhline(y=2/3, color='gray', linestyle='--', label='2/3 (theoretical for triple root)')
ax.set_xlabel("Iteration k", fontsize=12)
ax.set_ylabel("|e_k| / |e_{k-1}|", fontsize=12)
ax.set_title("Problem 3: Newton convergence ratio for f(x) = x³", fontsize=13)
ax.legend(fontsize=10)
ax.grid(True, ls="--", alpha=0.5)
fig.tight_layout()
fig.savefig("plot_newton_ratio_p3.png", dpi=150)
plt.close(fig)
print("Saved plot → plot_newton_ratio_p3.png")

print("\n✓ All computations and plots completed.")
