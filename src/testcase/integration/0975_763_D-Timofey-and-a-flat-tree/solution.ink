// Translated from solution.cpp.

var MOD = (cpp_cast(1e9) + 7);

var INF = cpp_cast(1e9);

var LINF = cpp_cast(1e18);

var PI = acos(cpp_cast(-1));

var EPS = 1e-9;

func gcd(a: dynamic, b: dynamic)
{
  var r: dynamic;
  while (b)
  {
    r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func fpow(n: dynamic, k: dynamic, p: dynamic = MOD)
{
  var r = 1;
  {
    while (k)
    {
      if ((k & 1))
      {
        r = ((r * n) % p);
      }
      n = ((n * n) % p);
      k >>= 1;
    }
  }
  return r;
}

func chkmin(a: dynamic, val: dynamic)
{
  return if ((val < a)) cpp_comma(cpp_assign(a, "=", val), 1) else 0;
}

func chkmax(a: dynamic, val: dynamic)
{
  return if ((a < val)) cpp_comma(cpp_assign(a, "=", val), 1) else 0;
}

func isqrt(k: dynamic)
{
  var r = (sqrt(k) + 1);
  while (((r * r) > k))
  {
    r -= 1;
  }
  return r;
}

func icbrt(k: dynamic)
{
  var r = (cbrt(k) + 1);
  while ((((r * r) * r) > k))
  {
    r -= 1;
  }
  return r;
}

func addmod(a: dynamic, val: dynamic, p: dynamic = MOD)
{
  if (((cpp_assign(a, "=", ((a + val)))) >= p))
  {
    a -= p;
  }
}

func submod(a: dynamic, val: dynamic, p: dynamic = MOD)
{
  if (((cpp_assign(a, "=", ((a - val)))) < 0))
  {
    a += p;
  }
}

func mult(a: dynamic, b: dynamic, p: dynamic = MOD)
{
  return ((cpp_cast(a) * b) % p);
}

func inv(a: dynamic, p: dynamic = MOD)
{
  return fpow(a, (p - 2), p);
}

func sign(x: dynamic)
{
  return (x + EPS);
}

func sign(x: dynamic, y: dynamic)
{
  return sign((x - y));
}

var maxn = (1000000 + 5);

var mod = (cpp_cast(1e8) + 7);

var n: dynamic;

var adj = cpp_array(maxn);

var hs: dynamic;

var f = cpp_array(maxn);

var g = cpp_array(maxn);

var cnt = cpp_array(maxn);

var tot: dynamic;

func query(val: dynamic)
{
  if (hs.count(val))
  {
    return hs[val];
  }
  var res = int_cpp((hs).size());
  return cpp_assign(hs[val], "=", res);
}

func add(val: dynamic)
{
  if ((!(cpp_update(cnt[val], "++"))))
  {
    tot += 1;
  }
}

func rem(val: dynamic)
{
  if ((!(cpp_update(cnt[val], "--"))))
  {
    tot -= 1;
  }
}

func dfs(u: dynamic, p: dynamic = -1)
{
  var r: dynamic;
  {
    var i = (0);
    while ((i < (int_cpp((adj[u]).size()))))
    {
      var v = adj[u][i];
      if ((v != p))
      {
        dfs(v, u);
        addmod(r.first, fpow(3, f[v]));
        addmod(r.second, fpow(5, f[v], mod), mod);
      }
      i += 1;
    }
  }
  add(cpp_assign(f[u], "=", query(r)));
}

func dfs2(u: dynamic, p: dynamic = -1, pv: dynamic = -1)
{
  var r: dynamic;
  if ((pv != -1))
  {
    addmod(r.first, fpow(3, pv));
    addmod(r.second, fpow(5, pv, mod), mod);
    add(pv);
  }
  {
    var i = (0);
    while ((i < (int_cpp((adj[u]).size()))))
    {
      var v = adj[u][i];
      if ((v != p))
      {
        addmod(r.first, fpow(3, f[v]));
        addmod(r.second, fpow(5, f[v], mod), mod);
      }
      i += 1;
    }
  }
  rem(f[u]);
  g[u] = tot;
  {
    var i = (0);
    while ((i < (int_cpp((adj[u]).size()))))
    {
      var v = adj[u][i];
      if ((v != p))
      {
        submod(r.first, fpow(3, f[v]));
        submod(r.second, fpow(5, f[v], mod), mod);
        dfs2(v, u, query(r));
        addmod(r.first, fpow(3, f[v]));
        addmod(r.second, fpow(5, f[v], mod), mod);
      }
      i += 1;
    }
  }
  if ((pv != -1))
  {
    rem(pv);
  }
  add(f[u]);
}

func solve()
{
  read(n);
  {
    var i = (0);
    while ((i < ((n - 1))))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      u -= 1;
      v -= 1;
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  dfs(0);
  dfs2(0);
  var best: dynamic;
  {
    var i = (0);
    while ((i < (n)))
    {
      chkmax(best, make_pair(g[i], i));
      i += 1;
    }
  }
  write((best.second + 1), "\n");
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  solve();
  return 0;
}
