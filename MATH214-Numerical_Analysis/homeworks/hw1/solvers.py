import math

TOL = 1e-6 # tolerance (ε = 10⁻⁶)
MAX_ITER = 100 # maximum number of iterations

# Bisection Method
def bisection(f, a, b, tol=TOL, max_iter=MAX_ITER):
    if f(a) * f(b) >= 0:
        raise ValueError(
            f"f(a) and f(b) must have opposite signs.  "
            f"f({a})={f(a):.6e}, f({b})={f(b):.6e}"
        )

    history = [a]
    prev = a
    for k in range(max_iter):
        c = (a + b) / 2.0
        history.append(c)
        fc = f(c)

        if abs(c - prev) < tol or abs(fc) < tol:
            return c, abs(fc), k + 1, history

        if f(a) * fc < 0:
            b = c
        else:
            a = c
        prev = c

    c = (a + b) / 2.0

    # Return the root, |f(root)|, number of iterations, and the history of approximations
    return c, abs(f(c)), max_iter, history


# Fixed-Point Iteration
def fixed_point(g, x0, f=None, tol=TOL, max_iter=MAX_ITER):
    history = [x0]
    x = x0
    for k in range(max_iter):
        try:
            x_new = g(x)
        except (OverflowError, ValueError, ZeroDivisionError):
            res = abs(f(x)) if f else float('inf')
            return x, res, k + 1, history

        history.append(x_new)

        fx_new = abs(f(x_new)) if f else float('inf')
        if abs(x_new - x) < tol or fx_new < tol:
            res = fx_new if f else abs(x_new - x)
            return x_new, res, k + 1, history

        if abs(x_new) > 1e15:
            res = abs(f(x_new)) if f else abs(x_new - x)
            return x_new, res, k + 1, history

        x = x_new

    res = abs(f(x)) if f else abs(x_new - x)

    # Return the root, |f(root)|, number of iterations, and the history of approximations
    return x, res, max_iter, history


# Newton's Method
def newton(f, df, x0, tol=TOL, max_iter=MAX_ITER):
    history = [x0]
    x = x0
    for k in range(max_iter):
        fx = f(x)
        dfx = df(x)

        if abs(dfx) < 1e-14:
            return x, abs(fx), k + 1, history

        x_new = x - fx / dfx
        history.append(x_new)

        if abs(x_new - x) < tol or abs(f(x_new)) < tol:
            return x_new, abs(f(x_new)), k + 1, history

        x = x_new

    # Return the root, |f(root)|, number of iterations, and the history of approximations
    return x, abs(f(x)), max_iter, history
