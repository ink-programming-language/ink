// Translated from solution.cpp.

var mod: dynamic = 1000000007;

class stree
{
  var t: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  func build(n: dynamic, v: dynamic, tl: dynamic, tr: dynamic) -> dynamic
  {
      if ((v == 1))
      {
        t.resize((n * 4));
        s = n;
      }
      if ((tl == tr))
      {
        t[v] = make_pair(0, 1);
      } else
      {
        var tm: dynamic = (((tl + tr)) / 2);
        build(n, (v * 2), tl, tm);
        build(n, ((v * 2) + 1), (tm + 1), tr);
        t[v].first = (t[(v * 2)].first + t[((v * 2) + 1)].first);
        t[v].second = (((t[(v * 2)].second * t[((v * 2) + 1)].second)) % mod);
      }
    }
  func set(v: dynamic, tl: dynamic, tr: dynamic, pos: dynamic, to: dynamic) -> dynamic
  {
      if (((pos == tl) && (tr == pos)))
      {
        t[v].first += to.first;
        t[v].second = (((to.second * t[v].second)) % mod);
      } else
      {
        var tm: dynamic = (((tl + tr)) / 2);
        if ((pos <= tm))
        {
          set((v * 2), tl, tm, pos, to);
        } else
        {
          set(((v * 2) + 1), (tm + 1), tr, pos, to);
        }
        t[v].first = (t[(v * 2)].first + t[((v * 2) + 1)].first);
        t[v].second = (((t[(v * 2)].second * t[((v * 2) + 1)].second)) % mod);
      }
    }
  func get(v: dynamic, tl: dynamic, tr: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if ((l > r))
      {
        return make_pair(0, 1);
      }
      if (((tl == l) && (tr == r)))
      {
        return t[v];
      }
      var tm: dynamic = (((tl + tr)) / 2);
      var t1: dynamic = get((v * 2), tl, tm, l, min(r, tm));
      var t2: dynamic = get(((v * 2) + 1), (tm + 1), tr, max(l, (tm + 1)), r);
      return make_pair((t1.first + t2.first), (((t1.second * t2.second)) % mod));
    }
}

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var xinv: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var ban: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var nb: dynamic = cpp_uninitialized();

var ch: dynamic = cpp_uninitialized();

var root: dynamic = cpp_uninitialized();

func gcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if ((a == 0))
  {
    x = 0;
    y = 1;
    return b;
  }
  var x1: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var d: dynamic = gcd((b % a), a, x1, y1);
  x = (y1 - (((b / a)) * x1));
  y = x1;
  return d;
}

func invmod(a: dynamic, modulo: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  gcd(a, modulo, x, y);
  return ((((x % modulo) + modulo)) % modulo);
}

func dfs(v: dynamic, p: dynamic, sz: dynamic) -> dynamic
{
  s[v] = 1;
  ch[v].clear();
  for (var u: dynamic in nb[v])
  {
    if ((ban[u.first] || (u.first == p)))
    {
      continue;
    }
    ch[v].push_back(u);
    dfs(u.first, v, sz);
    s[v] += s[u.first];
  }
  if ((((2 * s[v]) >= sz) && (root == -1)))
  {
    root = v;
  }
}

var e: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(2);

var cur: dynamic = cpp_uninitialized();

func dfse(v: dynamic, to: dynamic) -> dynamic
{
  to.push_back(make_pair(make_pair(cnt[0], cnt[1]), cur));
  for (var u: dynamic in ch[v])
  {
    cur = (((cur * x[u.second])) % mod);
    cnt[c[u.second]] += 1;
    dfse(u.first, to);
    cur = (((cur * xinv[u.second])) % mod);
    cnt[c[u.second]] -= 1;
  }
}

var vu2: dynamic = cpp_uninitialized();

var uv2: dynamic = cpp_uninitialized();

func powM(k: dynamic, n: dynamic, modulo: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return 1;
  }
  if ((n == 1))
  {
    return k;
  }
  var x: dynamic = powM(k, (n / 2), modulo);
  return (((((((x * x)) % modulo)) * powM(k, (n % 2), modulo))) % modulo);
}

var ans: dynamic = 1;

func proc(v: dynamic) -> dynamic
{
  dfs(v, -1, 0);
  root = -1;
  dfs(v, -1, s[v]);
  dfs(root, -1, 0);
  var total: dynamic = make_pair(0, 1);
  for (var u: dynamic in ch[root])
  {
    e[u.first].clear();
    cur = x[u.second];
    cnt[0] = 0;
    cnt[1] = 0;
    cnt[c[u.second]] += 1;
    dfse(u.first, e[u.first]);
    for (var ex: dynamic in e[u.first])
    {
      var p: dynamic = ex.first.first;
      var q: dynamic = ex.first.second;
      var val: dynamic = ex.second;
      var bad1: dynamic = uv2.get(1, 0, (3 * n), ((((2 * q) - p) + 1) + (2 * n)), (3 * n));
      var bad2: dynamic = vu2.get(1, 0, (3 * n), ((((2 * p) - q) + 1) + (2 * n)), (3 * n));
      ans = (((ans * powM(val, ((total.first - bad1.first) - bad2.first), mod))) % mod);
      ans = (((ans * total.second)) % mod);
      ans = (((ans * invmod(bad1.second, mod))) % mod);
      ans = (((ans * invmod(bad2.second, mod))) % mod);
    }
    for (var ex: dynamic in e[u.first])
    {
      var p: dynamic = ex.first.first;
      var q: dynamic = ex.first.second;
      var val: dynamic = ex.second;
      total.first += 1;
      total.second = (((total.second * val)) % mod);
      uv2.set(1, 0, (3 * n), ((p - (2 * q)) + (2 * n)), make_pair(1, val));
      vu2.set(1, 0, (3 * n), ((q - (2 * p)) + (2 * n)), make_pair(1, val));
    }
  }
  var bad1: dynamic = uv2.get(1, 0, (3 * n), (1 + (2 * n)), (3 * n));
  var bad2: dynamic = vu2.get(1, 0, (3 * n), (1 + (2 * n)), (3 * n));
  ans = (((ans * total.second)) % mod);
  ans = (((ans * invmod(bad1.second, mod))) % mod);
  ans = (((ans * invmod(bad2.second, mod))) % mod);
  for (var u: dynamic in ch[root])
  {
    for (var ex: dynamic in e[u.first])
    {
      var p: dynamic = ex.first.first;
      var q: dynamic = ex.first.second;
      var val: dynamic = ex.second;
      var ival: dynamic = invmod(val, mod);
      uv2.set(1, 0, (3 * n), ((p - (2 * q)) + (2 * n)), make_pair(-1, ival));
      vu2.set(1, 0, (3 * n), ((q - (2 * p)) + (2 * n)), make_pair(-1, ival));
    }
  }
  ban[root] = 1;
  for (var u: dynamic in nb[root])
  {
    if ((!ban[u.first]))
    {
      proc(u.first);
    }
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  vu2.build(((3 * n) + 1), 1, 0, (3 * n));
  uv2.build(((3 * n) + 1), 1, 0, (3 * n));
  x.resize(n);
  c.resize(n);
  ban.resize(n);
  nb.resize(n);
  e.resize(n);
  s.resize(n);
  ch.resize(n);
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d %d %lld %d", (&u), (&v), (&x[i]), (&c[i]));
      u -= 1;
      v -= 1;
      nb[v].push_back(make_pair(u, i));
      nb[u].push_back(make_pair(v, i));
      i += 1;
    }
  }
  for (var u: dynamic in x)
  {
    xinv.push_back(invmod(u, mod));
  }
  proc(0);
  write(ans);
}
