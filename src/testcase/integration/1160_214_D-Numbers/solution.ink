// Translated from solution.cpp.

func toInt(s: dynamic)
{
  var v: dynamic;
  (sin >> v);
  return v;
}

func toString(x: dynamic)
{
  var sout: dynamic;
  (sout << x);
  return sout.str();
}

func readInt()
{
  var x: dynamic;
  scanf("%d", (&x));
  return x;
}

var EPS = 1E-8;

class UnionFind
{
  var par: dynamic;
  var siz: dynamic;
  var maxv: dynamic;
  func UnionFind(sz: dynamic)
  {
      this->par = cpp_construct(sz);
      this->siz = cpp_construct(sz, 1);
      {
        var i = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func init(sz: dynamic)
  {
      par.resize(sz);
      siz.assign(sz, 1);
      {
        var i = 0;
        while ((i < sz))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func root(x: dynamic)
  {
      while ((par[x] != x))
      {
        x = cpp_assign(par[x], "=", par[par[x]]);
      }
      return x;
    }
  func merge(x: dynamic, y: dynamic)
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
  func issame(x: dynamic, y: dynamic)
  {
      return (root(x) == root(y));
    }
  func size(x: dynamic)
  {
      return siz[root(x)];
    }
}

func mod_pow(x: dynamic, n: dynamic, mod: dynamic)
{
  var res = 1;
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

var sieve = cpp_array((5000000 + 10));

func make_sieve()
{
  {
    var i = 0;
    while ((i < (5000000 + 10)))
    {
      sieve[i] = true;
      i += 1;
    }
  }
  sieve[0] = cpp_assign(sieve[1], "=", false);
  {
    var i = 2;
    while (((i * i) < (5000000 + 10)))
    {
      if (sieve[i])
      {
        {
          var j = 2;
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

func isprime(n: dynamic)
{
  if (((n == 0) || (n == 1)))
  {
    return false;
  }
  {
    var i = 2;
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

var MAX = 510000;

var fac = cpp_array(MAX);

var finv = cpp_array(MAX);

var inv = cpp_array(MAX);

func COMinit()
{
  fac[0] = cpp_assign(fac[1], "=", 1);
  finv[0] = cpp_assign(finv[1], "=", 1);
  inv[1] = 1;
  {
    var i = 2;
    while ((i < MAX))
    {
      fac[i] = ((fac[(i - 1)] * i) % 1000000007);
      inv[i] = (1000000007 - ((inv[(1000000007 % i)] * ((1000000007 / i))) % 1000000007));
      finv[i] = ((finv[(i - 1)] * inv[i]) % 1000000007);
      i += 1;
    }
  }
}

func COM(n: dynamic, k: dynamic)
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

func extGCD(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var d = extGCD(b, (a % b), y, x);
  y -= ((a / b) * x);
  return d;
}

func mod(a: dynamic, m: dynamic)
{
  return ((((a % m) + m)) % m);
}

func modinv(a: dynamic, m: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  extGCD(a, m, x, y);
  return mod(x, m);
}

func GCD(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return GCD(b, (a % b));
}

class LazySegmentTree
{
  var n: dynamic;
  var node: dynamic;
  var lazy: dynamic;
  func LazySegmentTree(v: dynamic)
  {
      var sz = cpp_cast(v.size());
      n = 1;
      while ((n < sz))
      {
        n *= 2;
      }
      node.resize(((2 * n) - 1));
      lazy.resize(((2 * n) - 1), 0);
      {
        var i = 0;
        while ((i < sz))
        {
          node[((i + n) - 1)] = v[i];
          i += 1;
        }
      }
      {
        var i = (n - 2);
        while ((i >= 0))
        {
          node[i] = (node[((i * 2) + 1)] + node[((i * 2) + 2)]);
          i -= 1;
        }
      }
    }
  func eval(k: dynamic, l: dynamic, r: dynamic)
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
  func add(a: dynamic, b: dynamic, x: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = -1)
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
  func getsum(a: dynamic, b: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = -1)
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
      var vl = getsum(a, b, ((2 * k) + 1), l, (((l + r)) / 2));
      var vr = getsum(a, b, ((2 * k) + 2), (((l + r)) / 2), r);
      return (vl + vr);
    }
}

class Edge
{
  var src: dynamic;
  var dst: dynamic;
  var weight: dynamic;
  var cap: dynamic;
  func Edge()
  {
      this->src = cpp_construct(0);
      this->dst = cpp_construct(0);
      this->weight = cpp_construct(0);
    }
  func Edge(s: dynamic, d: dynamic, w: dynamic)
  {
      this->src = cpp_construct(s);
      this->dst = cpp_construct(d);
      this->weight = cpp_construct(w);
    }
}

func add_edge(g: dynamic, a: dynamic, b: dynamic, w: dynamic = 1)
{
  g[a].emplace_back(a, b, w);
  g[b].emplace_back(b, a, w);
}

func add_arc(g: dynamic, a: dynamic, b: dynamic, w: dynamic = 1)
{
  g[a].emplace_back(a, b, w);
}

var n: dynamic;

var a = cpp_array(10);

var dp = cpp_array(11, 101);

var coef = cpp_array(101, 101);

func rec(n: dynamic, c: dynamic)
{
  if ((!n))
  {
    var ok = false;
    {
      var i = c;
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
    var i = a[c];
    while ((i <= n))
    {
      dp[n][c] = (((dp[n][c] + (coef[n][i] * rec((n - i), (c + 1))))) % 1000000007);
      i += 1;
    }
  }
  return dp[n][c];
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(n);
  var sum = 0;
  {
    var i = 0;
    while ((i < cpp_cast(10)))
    {
      read(a[i]);
      sum += a[i];
      i += 1;
    }
  }
  coef[0][0] = 1;
  {
    var i = 1;
    while ((i <= 100))
    {
      coef[i][0] = cpp_assign(coef[i][i], "=", 1);
      {
        var j = 1;
        while ((j < i))
        {
          coef[i][j] = (((coef[(i - 1)][j] + coef[(i - 1)][(j - 1)])) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var dig = 1;
    while ((dig <= n))
    {
      {
        var i = 1;
        while ((i < 10))
        {
          memset(dp, -1, cpp_sizeof((dp)));
          var u = (!(!a[i]));
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
