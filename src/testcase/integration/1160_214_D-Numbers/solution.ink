// Translated from solution.cpp.

func toInt(s: dynamic) -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  (sin >> v);
  return v;
}

func toString(x: dynamic) -> dynamic
{
  var sout: dynamic = cpp_uninitialized();
  (sout << x);
  return sout.str();
}

func readInt() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  scanf("%d", (&x));
  return x;
}

var EPS: dynamic = 1E-8;

class UnionFind
{
  var par: dynamic = cpp_uninitialized();
  var siz: dynamic = cpp_uninitialized();
  var maxv: dynamic = cpp_uninitialized();
  func UnionFind(sz: dynamic) -> dynamic
  {
      self->par = cpp_construct(sz);
      self->siz = cpp_construct(sz, 1);
      {
        var i: dynamic = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func init(sz: dynamic) -> dynamic
  {
      par.resize(sz);
      siz.assign(sz, 1);
      {
        var i: dynamic = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func root(x: dynamic) -> dynamic
  {
      while ((par[x] != x))
      {
        x = cpp_assign(par[x], "=", par[par[x]]);
      }
      return x;
    }
  func merge(x: dynamic, y: dynamic) -> dynamic
  {
      x = root(x);
      y = root(y);
      if ((x == y))
      {
        return false;
      }
      if ((siz[x] < siz[y]))
      {
        swap(x, y);
      }
      siz[x] += siz[y];
      par[y] = x;
      return true;
    }
  func issame(x: dynamic, y: dynamic) -> dynamic
  {
      return (root(x) == root(y));
    }
  func size(x: dynamic) -> dynamic
  {
      return siz[root(x)];
    }
}

func mod_pow(x: dynamic, n: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (n)
  {
    if ((n & 1))
    {
      res = (res * x);
    }
    res %= mod;
    x = ((x * x) % mod);
    n >>= 1;
  }
  return res;
}

var sieve: dynamic = cpp_array((5000000 + 10));

func make_sieve() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < (5000000 + 10)))
    {
      sieve[i] = true;
      i += 1;
    }
  }
  sieve[0] = cpp_assign(sieve[1], "=", false);
  {
    var i: dynamic = 2;
    while (((i * i) < (5000000 + 10)))
    {
      if (sieve[i])
      {
        {
          var j: dynamic = 2;
          while (((i * j) < (5000000 + 10)))
          {
            sieve[(i * j)] = false;
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
}

func isprime(n: dynamic) -> dynamic
{
  if (((n == 0) || (n == 1)))
  {
    return false;
  }
  {
    var i: dynamic = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

var MAX: dynamic = 510000;

var fac: dynamic = cpp_array(MAX);

var finv: dynamic = cpp_array(MAX);

var inv: dynamic = cpp_array(MAX);

func COMinit() -> dynamic
{
  fac[0] = cpp_assign(fac[1], "=", 1);
  finv[0] = cpp_assign(finv[1], "=", 1);
  inv[1] = 1;
  {
    var i: dynamic = 2;
    while ((i < MAX))
    {
      fac[i] = ((fac[(i - 1)] * i) % 1000000007);
      inv[i] = (1000000007 - ((inv[(1000000007 % i)] * ((1000000007 / i))) % 1000000007));
      finv[i] = ((finv[(i - 1)] * inv[i]) % 1000000007);
      i += 1;
    }
  }
}

func COM(n: dynamic, k: dynamic) -> dynamic
{
  if ((n < k))
  {
    return 0;
  }
  if (((n < 0) || (k < 0)))
  {
    return 0;
  }
  return ((fac[n] * (((finv[k] * finv[(n - k)]) % 1000000007))) % 1000000007);
}

func extGCD(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var d: dynamic = extGCD(b, (a % b), y, x);
  y -= ((a / b) * x);
  return d;
}

func mod(a: dynamic, m: dynamic) -> dynamic
{
  return ((((a % m) + m)) % m);
}

func modinv(a: dynamic, m: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  extGCD(a, m, x, y);
  return mod(x, m);
}

func GCD(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return GCD(b, (a % b));
}

class LazySegmentTree
{
  var n: dynamic = cpp_uninitialized();
  var node: dynamic = cpp_uninitialized();
  var lazy: dynamic = cpp_uninitialized();
  func LazySegmentTree(v: dynamic) -> dynamic
  {
      var sz: dynamic = cpp_cast(v.size());
      n = 1;
      while ((n < sz))
      {
        n *= 2;
      }
      node.resize(((2 * n) - 1));
      lazy.resize(((2 * n) - 1), 0);
      {
        var i: dynamic = 0;
        while ((i < sz))
        {
          node[((i + n) - 1)] = v[i];
          i += 1;
        }
      }
      {
        var i: dynamic = (n - 2);
        while ((i >= 0))
        {
          node[i] = (node[((i * 2) + 1)] + node[((i * 2) + 2)]);
          i -= 1;
        }
      }
    }
  func eval(k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if ((lazy[k] != 0))
      {
        node[k] += lazy[k];
        if (((r - l) > 1))
        {
          lazy[((2 * k) + 1)] += (lazy[k] / 2);
          lazy[((2 * k) + 2)] += (lazy[k] / 2);
        }
        lazy[k] = 0;
      }
    }
  func add(a: dynamic, b: dynamic, x: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = -1) -> dynamic
  {
      if ((r < 0))
      {
        r = n;
      }
      eval(k, l, r);
      if (((b <= l) || (r <= a)))
      {
        return;
      }
      if (((a <= l) && (r <= b)))
      {
        lazy[k] += (((r - l)) * x);
        eval(k, l, r);
      } else
      {
        add(a, b, x, ((2 * k) + 1), l, (((l + r)) / 2));
        add(a, b, x, ((2 * k) + 2), (((l + r)) / 2), r);
        node[k] = (node[((2 * k) + 1)] + node[((2 * k) + 2)]);
      }
    }
  func getsum(a: dynamic, b: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = -1) -> dynamic
  {
      if ((r < 0))
      {
        r = n;
      }
      eval(k, l, r);
      if (((b <= l) || (r <= a)))
      {
        return 0;
      }
      if (((a <= l) && (r <= b)))
      {
        return node[k];
      }
      var vl: dynamic = getsum(a, b, ((2 * k) + 1), l, (((l + r)) / 2));
      var vr: dynamic = getsum(a, b, ((2 * k) + 2), (((l + r)) / 2), r);
      return (vl + vr);
    }
}

class Edge
{
  var src: dynamic = cpp_uninitialized();
  var dst: dynamic = cpp_uninitialized();
  var weight: dynamic = cpp_uninitialized();
  var cap: dynamic = cpp_uninitialized();
  func Edge() -> dynamic
  {
      self->src = cpp_construct(0);
      self->dst = cpp_construct(0);
      self->weight = cpp_construct(0);
    }
  func Edge(s: dynamic, d: dynamic, w: dynamic) -> dynamic
  {
      self->src = cpp_construct(s);
      self->dst = cpp_construct(d);
      self->weight = cpp_construct(w);
    }
}

func add_edge(g: dynamic, a: dynamic, b: dynamic, w: dynamic = 1) -> dynamic
{
  g[a].emplace_back(a, b, w);
  g[b].emplace_back(b, a, w);
}

func add_arc(g: dynamic, a: dynamic, b: dynamic, w: dynamic = 1) -> dynamic
{
  g[a].emplace_back(a, b, w);
}

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(10);

var dp: dynamic = cpp_array(11, 101);

var coef: dynamic = cpp_array(101, 101);

func rec(n: dynamic, c: dynamic) -> dynamic
{
  if ((!n))
  {
    var ok: dynamic = false;
    {
      var i: dynamic = c;
      while ((i < 10))
      {
        ok |= (!(!a[i]));
        i += 1;
      }
    }
    return (!ok);
  }
  if ((c == 10))
  {
    return 0;
  }
  if ((~dp[n][c]))
  {
    return dp[n][c];
  }
  dp[n][c] = 0;
  {
    var i: dynamic = a[c];
    while ((i <= n))
    {
      dp[n][c] = (((dp[n][c] + (coef[n][i] * rec((n - i), (c + 1))))) % 1000000007);
      i += 1;
    }
  }
  return dp[n][c];
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(n);
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(10)))
    {
      read(a[i]);
      sum += a[i];
      i += 1;
    }
  }
  coef[0][0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= 100))
    {
      coef[i][0] = cpp_assign(coef[i][i], "=", 1);
      {
        var j: dynamic = 1;
        while ((j < i))
        {
          coef[i][j] = (((coef[(i - 1)][j] + coef[(i - 1)][(j - 1)])) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var dig: dynamic = 1;
    while ((dig <= n))
    {
      {
        var i: dynamic = 1;
        while ((i < 10))
        {
          memset(dp, -1, cpp_sizeof((dp)));
          var u: dynamic = (!(!a[i]));
          a[i] -= u;
          ans = (((ans + rec((dig - 1), 0))) % 1000000007);
          a[i] += u;
          i += 1;
        }
      }
      dig += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
