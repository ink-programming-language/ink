// Translated from solution.cpp.

var N = 300200;

var n: dynamic;

var tim: dynamic;

var tin = cpp_array(N);

var tout = cpp_array(N);

var dep = cpp_array(N);

var mod = (1e9 + 7);

var t = cpp_array((4 * N));

var v = cpp_array(N);

func dfs(x: dynamic, g: dynamic)
{
  tin[x] = cpp_assign(tout[x], "=", cpp_update(tim, "++"));
  dep[tin[x]] = g;
  for (var y in v[x])
  {
    dfs(y, (g - 1));
    tout[x] = tout[y];
  }
}

func push(x: dynamic)
{
  t[(x * 2)].first += t[x].first;
  t[(x * 2)].second += t[x].second;
  t[((x * 2) + 1)].first += t[x].first;
  t[((x * 2) + 1)].second += t[x].second;
  t[(x * 2)].first %= mod;
  t[(x * 2)].second %= mod;
  t[((x * 2) + 1)].first %= mod;
  t[((x * 2) + 1)].second %= mod;
  t[x].first = cpp_assign(t[x].second, "=", 0);
}

func upd(x: dynamic, l: dynamic, r: dynamic, tl: dynamic, tr: dynamic, f1: dynamic, f2: dynamic)
{
  if ((tl > tr))
  {
    return;
  }
  if (((l == tl) && (r == tr)))
  {
    t[x].first = (((t[x].first + f1)) % mod);
    t[x].second = (((t[x].second + f2)) % mod);
    return;
  }
  push(x);
  var m = (((l + r)) / 2);
  upd((x * 2), l, m, tl, min(m, tr), f1, f2);
  upd(((x * 2) + 1), (m + 1), r, max((m + 1), tl), tr, f1, f2);
}

func get(x: dynamic, l: dynamic, r: dynamic, g: dynamic)
{
  if ((l == r))
  {
    var ans = ((((1 * t[x].second) * dep[l])) % mod);
    ans = (((t[x].first + ans)) % mod);
    return ans;
  }
  push(x);
  var m = (((l + r)) / 2);
  if ((g <= m))
  {
    return get((x * 2), l, m, g);
  } else
  {
    return get(((x * 2) + 1), (m + 1), r, g);
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  scanf("%d", (&n));
  {
    var i = 2;
    while ((i <= n))
    {
      var x: dynamic;
      scanf("%d", (&x));
      v[x].push_back(i);
      i += 1;
    }
  }
  dfs(1, n);
  var q: dynamic;
  scanf("%d", (&q));
  {
    var i = 1;
    while ((i <= q))
    {
      var t: dynamic;
      var v: dynamic;
      var x: dynamic;
      var k: dynamic;
      scanf("%d", (&t));
      if ((t == 1))
      {
        scanf("%d%d%d", (&v), (&x), (&k));
        var f = ((1 * x) - ((1 * dep[tin[v]]) * k));
        f = ((f % mod) + mod);
        upd(1, 1, n, tin[v], tout[v], (f % mod), k);
      } else
      {
        scanf("%d", (&v));
        printf("%d\n", get(1, 1, n, tin[v]));
      }
      i += 1;
    }
  }
}
