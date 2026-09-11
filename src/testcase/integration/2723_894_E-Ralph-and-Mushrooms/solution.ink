// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var NINF: dynamic = 0xc0c0c0c0;

var maxn: dynamic = (1e6 + 5);

class Edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
}

var edges: dynamic = cpp_array(maxn);

var dp: dynamic = cpp_array(maxn);

var val: dynamic = cpp_array(maxn);

var scc: dynamic = cpp_uninitialized();

var Index: dynamic = cpp_uninitialized();

var top: dynamic = cpp_uninitialized();

var G_new: dynamic = cpp_array(maxn);

var G: dynamic = cpp_array(maxn);

var Stack: dynamic = cpp_array(maxn);

var low: dynamic = cpp_array(maxn);

var dfn: dynamic = cpp_array(maxn);

var belong: dynamic = cpp_array(maxn);

var maps: dynamic = cpp_array(maxn);

var cost: dynamic = cpp_array(maxn);

var instack: dynamic = cpp_array(maxn);

var vis: dynamic = cpp_array(maxn);

func Tarjan(u: dynamic) -> dynamic
{
  dfn[u] = cpp_assign(low[u], "=", cpp_update(Index, "++"));
  instack[u] = true;
  Stack[cpp_update(top, "++")] = u;
  {
    var i: dynamic = 0;
    while ((i < G[u].size()))
    {
      var v: dynamic = G[u][i];
      if ((!dfn[v]))
      {
        Tarjan(v);
        low[u] = min(low[u], low[v]);
      } else if (instack[v])
      {
        low[u] = min(low[u], dfn[v]);
      }
      i += 1;
    }
  }
  if ((dfn[u] == low[u]))
  {
    scc += 1;
    while ((top > 0))
    {
      var now: dynamic = Stack[cpp_update(top, "--")];
      belong[now] = u;
      instack[now] = false;
      if ((now == u))
      {
        maps[u] = scc;
        break;
      }
    }
  }
}

func solve(n: dynamic) -> dynamic
{
  memset(dfn, 0, cpp_sizeof((dfn)));
  memset(instack, 0, cpp_sizeof((instack)));
  scc = cpp_assign(Index, "=", cpp_assign(top, "=", 0));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!dfn[i]))
      {
        Tarjan(i);
      }
      i += 1;
    }
  }
}

func dfs(pos: dynamic) -> dynamic
{
  if ((~dp[pos]))
  {
    return dp[pos];
  }
  dp[pos] = 0;
  for (var e: dynamic in G_new[pos])
  {
    var v: dynamic = e.first;
    var w: dynamic = e.second;
    var temp: dynamic = (dfs(v) + w);
    if ((dp[pos] < temp))
    {
      dp[pos] = temp;
    }
  }
  return cpp_assign(dp[pos], "+=", cost[pos]);
}

func init() -> dynamic
{
  val[0] = 0;
  var temp: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      temp += i;
      val[i] = (val[(i - 1)] + temp);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  init();
  while ((~scanf("%d%d", (&n), (&m))))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        G[i].clear();
        G_new[i].clear();
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        var u: dynamic = cpp_uninitialized();
        var v: dynamic = cpp_uninitialized();
        var w: dynamic = cpp_uninitialized();
        scanf("%d%d%d", (&u), (&v), (&w));
        edges[i] = [u, v, w];
        G[edges[i].u].push_back(edges[i].v);
        i += 1;
      }
    }
    solve(n);
    memset(dp, -1, cpp_sizeof((dp)));
    memset(cost, 0, cpp_sizeof((cost)));
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        var u: dynamic = cpp_uninitialized();
        var v: dynamic = cpp_uninitialized();
        var w: dynamic = cpp_uninitialized();
        u = maps[belong[edges[i].u]];
        v = maps[belong[edges[i].v]];
        w = edges[i].w;
        if ((u != v))
        {
          G_new[u].push_back(make_pair(v, w));
        } else
        {
          var l: dynamic = 0;
          var r: dynamic = w;
          var pos: dynamic = cpp_uninitialized();
          while ((l <= r))
          {
            var mid: dynamic = (((l + r)) >> 1);
            if ((((((mid + 1)) * mid) / 2) <= w))
            {
              pos = mid;
              l = (mid + 1);
            } else
            {
              r = (mid - 1);
            }
          }
          cost[u] += ((cpp_cast(w) * ((pos + 1))) - val[pos]);
        }
        i += 1;
      }
    }
    memset(vis, 0, cpp_sizeof((vis)));
    scanf("%d", (&s));
    s = maps[belong[s]];
    printf("%I64d\n", dfs(s));
  }
}
