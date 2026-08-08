// Translated from solution.cpp.

var mod = 1000000007;

class stree
{
  var t: dynamic;
  var s: dynamic;
  func build(n: dynamic, v: dynamic, tl: dynamic, tr: dynamic)
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
        var tm = (((tl + tr)) / 2);
        build(n, (v * 2), tl, tm);
        build(n, ((v * 2) + 1), (tm + 1), tr);
        t[v].first = (t[(v * 2)].first + t[((v * 2) + 1)].first);
        t[v].second = (((t[(v * 2)].second * t[((v * 2) + 1)].second)) % mod);
      }
    }
  func set(v: dynamic, tl: dynamic, tr: dynamic, pos: dynamic, to: dynamic)
  {
      if (((pos == tl) && (tr == pos)))
      {
        t[v].first += to.first;
        t[v].second = (((to.second * t[v].second)) % mod);
      } else
      {
        var tm = (((tl + tr)) / 2);
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
  func get(v: dynamic, tl: dynamic, tr: dynamic, l: dynamic, r: dynamic)
  {
      if ((l > r))
      {
        return make_pair(0, 1);
      }
      if (((tl == l) && (tr == r)))
      {
        return t[v];
      }
      var tm = (((tl + tr)) / 2);
      var t1 = get((v * 2), tl, tm, l, min(r, tm));
      var t2 = get(((v * 2) + 1), (tm + 1), tr, max(l, (tm + 1)), r);
      return make_pair((t1.first + t2.first), (((t1.second * t2.second)) % mod));
    }
}

var n: dynamic;

var x: dynamic;

var xinv: dynamic;

var c: dynamic;

var ban: dynamic;

var s: dynamic;

var nb: dynamic;

var ch: dynamic;

var root: dynamic;

func gcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((a == 0))
  {
    x = 0;
    y = 1;
    return b;
  }
  var x1: dynamic;
  var y1: dynamic;
  var d = gcd((b % a), a, x1, y1);
  x = (y1 - (((b / a)) * x1));
  y = x1;
  return d;
}

func invmod(a: dynamic, modulo: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  gcd(a, modulo, x, y);
  return ((((x % modulo) + modulo)) % modulo);
}

func dfs(v: dynamic, p: dynamic, sz: dynamic)
{
  s[v] = 1;
  ch[v].clear();
  for (var u in nb[v])
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

var e: dynamic;

var cnt = cpp_array(2);

var cur: dynamic;

func dfse(v: dynamic, to: dynamic)
{
  to.push_back(make_pair(make_pair(cnt[0], cnt[1]), cur));
  for (var u in ch[v])
  {
    cur = (((cur * x[u.second])) % mod);
    cnt[c[u.second]] += 1;
    dfse(u.first, to);
    cur = (((cur * xinv[u.second])) % mod);
    cnt[c[u.second]] -= 1;
  }
}

var vu2: dynamic;

var uv2: dynamic;

func powM(k: dynamic, n: dynamic, modulo: dynamic)
{
  if ((n == 0))
  {
    return 1;
  }
  if ((n == 1))
  {
    return k;
  }
  var x = powM(k, (n / 2), modulo);
  return (((((((x * x)) % modulo)) * powM(k, (n % 2), modulo))) % modulo);
}

var ans = 1;

func proc(v: dynamic)
{
  dfs(v, -1, 0);
  root = -1;
  dfs(v, -1, s[v]);
  dfs(root, -1, 0);
  var total = make_pair(0, 1);
  for (var u in ch[root])
  {
    e[u.first].clear();
    cur = x[u.second];
    cnt[0] = 0;
    cnt[1] = 0;
    cnt[c[u.second]] += 1;
    dfse(u.first, e[u.first]);
    for (var ex in e[u.first])
    {
      var p = ex.first.first;
      var q = ex.first.second;
      var val = ex.second;
      var bad1 = uv2.get(1, 0, (3 * n), ((((2 * q) - p) + 1) + (2 * n)), (3 * n));
      var bad2 = vu2.get(1, 0, (3 * n), ((((2 * p) - q) + 1) + (2 * n)), (3 * n));
      ans = (((ans * powM(val, ((total.first - bad1.first) - bad2.first), mod))) % mod);
      ans = (((ans * total.second)) % mod);
      ans = (((ans * invmod(bad1.second, mod))) % mod);
      ans = (((ans * invmod(bad2.second, mod))) % mod);
    }
    for (var ex in e[u.first])
    {
      var p = ex.first.first;
      var q = ex.first.second;
      var val = ex.second;
      total.first += 1;
      total.second = (((total.second * val)) % mod);
      uv2.set(1, 0, (3 * n), ((p - (2 * q)) + (2 * n)), make_pair(1, val));
      vu2.set(1, 0, (3 * n), ((q - (2 * p)) + (2 * n)), make_pair(1, val));
    }
  }
  var bad1 = uv2.get(1, 0, (3 * n), (1 + (2 * n)), (3 * n));
  var bad2 = vu2.get(1, 0, (3 * n), (1 + (2 * n)), (3 * n));
  ans = (((ans * total.second)) % mod);
  ans = (((ans * invmod(bad1.second, mod))) % mod);
  ans = (((ans * invmod(bad2.second, mod))) % mod);
  for (var u in ch[root])
  {
    for (var ex in e[u.first])
    {
      var p = ex.first.first;
      var q = ex.first.second;
      var val = ex.second;
      var ival = invmod(val, mod);
      uv2.set(1, 0, (3 * n), ((p - (2 * q)) + (2 * n)), make_pair(-1, ival));
      vu2.set(1, 0, (3 * n), ((q - (2 * p)) + (2 * n)), make_pair(-1, ival));
    }
  }
  ban[root] = 1;
  for (var u in nb[root])
  {
    if ((!ban[u.first]))
    {
      proc(u.first);
    }
  }
}

func main()
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
    var i = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d %d %lld %d", (&u), (&v), (&x[i]), (&c[i]));
      u -= 1;
      v -= 1;
      nb[v].push_back(make_pair(u, i));
      nb[u].push_back(make_pair(v, i));
      i += 1;
    }
  }
  for (var u in x)
  {
    xinv.push_back(invmod(u, mod));
  }
  proc(0);
  write(ans);
}
