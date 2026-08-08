// Translated from solution.cpp.

var N = (1e5 + 5);

var Mod = (1e9 + 7);

var MAXN = 1e5;

var Lg = 27;

var M = (1e7 + 10);

var P = 727;

var Sq = 320;

var inf = (3e18 + 10);

var seg = cpp_array((4 * N));

var lz = cpp_array((4 * N));

var Par = cpp_array(Lg, N);

var St = cpp_array(N);

var Ed = cpp_array(N);

var H = cpp_array(N);

var Ans = cpp_array(N);

var Tim = 1;

var n: dynamic;

var q: dynamic;

var Adj = cpp_array(N);

var Q = cpp_array(N);

func Shift(s: dynamic, e: dynamic, ind: dynamic)
{
  var x = lz[ind];
  seg[((2 * (ind)))] += x;
  seg[((((2 * (ind))) + 1))] += x;
  lz[((2 * (ind)))] += x;
  lz[((((2 * (ind))) + 1))] += x;
  lz[ind] = 0;
}

func Add(s: dynamic, e: dynamic, ind: dynamic, l: dynamic, r: dynamic, x: dynamic)
{
  if (((s >= l) && (e <= r)))
  {
    seg[ind] += x;
    lz[ind] += x;
    return;
  }
  Shift(s, e, ind);
  var mid = (((s + e)) / 2);
  if ((l < mid))
  {
    Add(s, mid, ((2 * (ind))), l, r, x);
  }
  if ((r > mid))
  {
    Add(mid, e, ((((2 * (ind))) + 1)), l, r, x);
  }
  seg[ind] = max(seg[((2 * (ind)))], seg[((((2 * (ind))) + 1))]);
}

func Max(s: dynamic, e: dynamic, ind: dynamic, l: dynamic, r: dynamic)
{
  if (((s >= l) && (e <= r)))
  {
    return seg[ind];
  }
  Shift(s, e, ind);
  var mid = (((s + e)) / 2);
  var ret = (-Mod);
  if ((l < mid))
  {
    ret = max(ret, Max(s, mid, ((2 * (ind))), l, r));
  }
  if ((r > mid))
  {
    ret = max(ret, Max(mid, e, ((((2 * (ind))) + 1)), l, r));
  }
  return ret;
}

func Dfs(u: dynamic, par: dynamic)
{
  Par[u][0] = par;
  {
    var i = 1;
    while ((i < Lg))
    {
      Par[u][i] = Par[Par[u][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  St[u] = cpp_update(Tim, "++");
  Add(1, N, 1, St[u], (St[u] + 1), H[u]);
  {
    var i = 0;
    while ((i < cpp_cast(Adj[u].size())))
    {
      var x = Adj[u][i];
      if ((x == par))
      {
        i += 1;
        continue;
      }
      H[x] = (H[u] + 1);
      Dfs(x, u);
      i += 1;
    }
  }
  Ed[u] = Tim;
}

func Find(u: dynamic, x: dynamic)
{
  while (x)
  {
    var ct = builtin_ctz(x);
    u = Par[u][ct];
    x = ((x ^ ((1 << ct))));
  }
  return u;
}

func Lca(u: dynamic, v: dynamic)
{
  if ((H[u] > H[v]))
  {
    swap(u, v);
  }
  v = Find(v, (H[v] - H[u]));
  if ((u == v))
  {
    return u;
  }
  {
    var i = (Lg - 1);
    while ((i >= 0))
    {
      if ((Par[u][i] != Par[v][i]))
      {
        u = Par[u][i];
        v = Par[v][i];
      }
      i -= 1;
    }
  }
  return Par[u][0];
}

func Recalc(x: dynamic, y: dynamic)
{
  if ((St[x] < St[y]))
  {
    Add(1, N, 1, St[y], Ed[y], -1);
    if ((St[y] > 1))
    {
      Add(1, N, 1, 1, St[y], 1);
    }
    if ((Ed[y] < N))
    {
      Add(1, N, 1, Ed[y], N, 1);
    }
  } else
  {
    Add(1, N, 1, St[x], Ed[x], 1);
    if ((St[x] > 1))
    {
      Add(1, N, 1, 1, St[x], -1);
    }
    if ((Ed[x] < N))
    {
      Add(1, N, 1, Ed[x], N, -1);
    }
  }
}

func Solve(u: dynamic, par: dynamic)
{
  {
    var i = 0;
    while ((i < cpp_cast(Q[u].size())))
    {
      var x = Q[u][i].first;
      var y = u;
      var lc = Lca(x, y);
      var dis = ((((H[x] + H[y])) - (2 * (H[lc]))) - 1);
      var ret = 0;
      var cur = ((dis / 2));
      var nxt = 0;
      if ((u > x))
      {
        cur = (((dis + 1)) / 2);
      }
      if (((H[u] < H[x]) || (((H[u] == H[x]) && (u > x)))))
      {
        nxt = Find(x, (dis - cur));
        if ((St[nxt] > 1))
        {
          ret = max(ret, Max(1, N, 1, 1, St[nxt]));
        }
        if ((Ed[nxt] < N))
        {
          ret = max(ret, Max(1, N, 1, Ed[nxt], (n + 1)));
        }
      } else
      {
        nxt = Find(u, cur);
        ret = max(ret, Max(1, N, 1, St[nxt], Ed[nxt]));
      }
      Ans[Q[u][i].second] = max(Ans[Q[u][i].second], ret);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast(Adj[u].size())))
    {
      var x = Adj[u][i];
      if ((x == par))
      {
        i += 1;
        continue;
      }
      Recalc(u, x);
      Solve(x, u);
      Recalc(x, u);
      i += 1;
    }
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= (n - 1)))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d", (&x), (&y));
      Adj[x].push_back(y);
      Adj[y].push_back(x);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < Lg))
    {
      Par[1][i] = 1;
      i += 1;
    }
  }
  Dfs(1, 1);
  scanf("%d", (&q));
  {
    var i = 1;
    while ((i <= q))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d", (&x), (&y));
      Q[x].push_back(make_pair(y, i));
      Q[y].push_back(make_pair(x, i));
      i += 1;
    }
  }
  Solve(1, 1);
  {
    var i = 1;
    while ((i <= q))
    {
      printf("%d\n", Ans[i]);
      i += 1;
    }
  }
  return 0;
}
