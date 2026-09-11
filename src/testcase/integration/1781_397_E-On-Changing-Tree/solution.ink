// Translated from solution.cpp.

var N: dynamic = 300200;

var n: dynamic = cpp_uninitialized();

var tim: dynamic = cpp_uninitialized();

var tin: dynamic = cpp_array(N);

var tout: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

var mod: dynamic = (1e9 + 7);

var t: dynamic = cpp_array((4 * N));

var v: dynamic = cpp_array(N);

func dfs(x: dynamic, g: dynamic) -> dynamic
{
  tin[x] = cpp_assign(tout[x], "=", cpp_update(tim, "++"));
  dep[tin[x]] = g;
  for (var y: dynamic in v[x])
  {
    dfs(y, (g - 1));
    tout[x] = tout[y];
  }
}

func push(x: dynamic) -> dynamic
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

func upd(x: dynamic, l: dynamic, r: dynamic, tl: dynamic, tr: dynamic, f1: dynamic, f2: dynamic) -> dynamic
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
  var m: dynamic = (((l + r)) / 2);
  upd((x * 2), l, m, tl, min(m, tr), f1, f2);
  upd(((x * 2) + 1), (m + 1), r, max((m + 1), tl), tr, f1, f2);
}

func get(x: dynamic, l: dynamic, r: dynamic, g: dynamic) -> dynamic
{
  if ((l == r))
  {
    var ans: dynamic = ((((1 * t[x].second) * dep[l])) % mod);
    ans = (((t[x].first + ans)) % mod);
    return ans;
  }
  push(x);
  var m: dynamic = (((l + r)) / 2);
  if ((g <= m))
  {
    return get((x * 2), l, m, g);
  } else
  {
    return get(((x * 2) + 1), (m + 1), r, g);
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  scanf("%d", (&n));
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      v[x].push_back(i);
      i += 1;
    }
  }
  dfs(1, n);
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      var t: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      var k: dynamic = cpp_uninitialized();
      scanf("%d", (&t));
      if ((t == 1))
      {
        scanf("%d%d%d", (&v), (&x), (&k));
        var f: dynamic = ((1 * x) - ((1 * dep[tin[v]]) * k));
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
