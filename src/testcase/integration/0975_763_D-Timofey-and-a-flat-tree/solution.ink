// Translated from solution.cpp.

var MOD: dynamic = (cpp_cast(1e9) + 7);

var INF: dynamic = cpp_cast(1e9);

var LINF: dynamic = cpp_cast(1e18);

var PI: dynamic = acos(cpp_cast(-1));

var EPS: dynamic = 1e-9;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  while (b)
  {
    r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / gcd(a, b)) * b);
}

func fpow(n: dynamic, k: dynamic, p: dynamic = MOD) -> dynamic
{
  var r: dynamic = 1;
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

func chkmin(a: dynamic, val: dynamic) -> dynamic
{
  return  ((val < a)) ? cpp_comma(cpp_assign(a, "=", val), 1) : 0;
}

func chkmax(a: dynamic, val: dynamic) -> dynamic
{
  return  ((a < val)) ? cpp_comma(cpp_assign(a, "=", val), 1) : 0;
}

func isqrt(k: dynamic) -> dynamic
{
  var r: dynamic = (sqrt(k) + 1);
  while (((r * r) > k))
  {
    r -= 1;
  }
  return r;
}

func icbrt(k: dynamic) -> dynamic
{
  var r: dynamic = (cbrt(k) + 1);
  while ((((r * r) * r) > k))
  {
    r -= 1;
  }
  return r;
}

func addmod(a: dynamic, val: dynamic, p: dynamic = MOD) -> dynamic
{
  if (((cpp_assign(a, "=", ((a + val)))) >= p))
  {
    a -= p;
  }
}

func submod(a: dynamic, val: dynamic, p: dynamic = MOD) -> dynamic
{
  if (((cpp_assign(a, "=", ((a - val)))) < 0))
  {
    a += p;
  }
}

func mult(a: dynamic, b: dynamic, p: dynamic = MOD) -> dynamic
{
  return ((cpp_cast(a) * b) % p);
}

func inv(a: dynamic, p: dynamic = MOD) -> dynamic
{
  return fpow(a, (p - 2), p);
}

func sign(x: dynamic) -> dynamic
{
  return (x + EPS);
}

func sign(x: dynamic, y: dynamic) -> dynamic
{
  return sign((x - y));
}

var maxn: dynamic = (1000000 + 5);

var mod: dynamic = (cpp_cast(1e8) + 7);

var n: dynamic = cpp_uninitialized();

var adj: dynamic = cpp_array(maxn);

var hs: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(maxn);

var g: dynamic = cpp_array(maxn);

var cnt: dynamic = cpp_array(maxn);

var tot: dynamic = cpp_uninitialized();

func query(val: dynamic) -> dynamic
{
  if (hs.count(val))
  {
    return hs[val];
  }
  var res: dynamic = int_cpp((hs).size());
  return cpp_assign(hs[val], "=", res);
}

func add(val: dynamic) -> dynamic
{
  if ((!(cpp_update(cnt[val], "++"))))
  {
    tot += 1;
  }
}

func rem(val: dynamic) -> dynamic
{
  if ((!(cpp_update(cnt[val], "--"))))
  {
    tot -= 1;
  }
}

func dfs(u: dynamic, p: dynamic = -1) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  {
    var i: dynamic = (0);
    while ((i < (int_cpp((adj[u]).size()))))
    {
      var v: dynamic = adj[u][i];
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

func dfs2(u: dynamic, p: dynamic = -1, pv: dynamic = -1) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  if ((pv != -1))
  {
    addmod(r.first, fpow(3, pv));
    addmod(r.second, fpow(5, pv, mod), mod);
    add(pv);
  }
  {
    var i: dynamic = (0);
    while ((i < (int_cpp((adj[u]).size()))))
    {
      var v: dynamic = adj[u][i];
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
    var i: dynamic = (0);
    while ((i < (int_cpp((adj[u]).size()))))
    {
      var v: dynamic = adj[u][i];
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

func solve() -> dynamic
{
  read(n);
  {
    var i: dynamic = (0);
    while ((i < ((n - 1))))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
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
  var best: dynamic = cpp_uninitialized();
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      chkmax(best, make_pair(g[i], i));
      i += 1;
    }
  }
  write((best.second + 1), "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  solve();
  return 0;
}
