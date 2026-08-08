// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var NINF = 0xc0c0c0c0;

var maxn = (1e6 + 5);

class Edge
{
  var u: dynamic;
  var v: dynamic;
  var w: dynamic;
}

var edges = cpp_array(maxn);

var dp = cpp_array(maxn);

var val = cpp_array(maxn);

var scc: dynamic;

var Index: dynamic;

var top: dynamic;

var G_new = cpp_array(maxn);

var G = cpp_array(maxn);

var Stack = cpp_array(maxn);

var low = cpp_array(maxn);

var dfn = cpp_array(maxn);

var belong = cpp_array(maxn);

var maps = cpp_array(maxn);

var cost = cpp_array(maxn);

var instack = cpp_array(maxn);

var vis = cpp_array(maxn);

func Tarjan(u: dynamic)
{
  dfn[u] = cpp_assign(low[u], "=", cpp_update(Index, "++"));
  instack[u] = true;
  Stack[cpp_update(top, "++")] = u;
  {
    var i = 0;
    while ((i < G[u].size()))
    {
      var v = G[u][i];
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
      var now = Stack[cpp_update(top, "--")];
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

func solve(n: dynamic)
{
  memset(dfn, 0, cpp_sizeof((dfn)));
  memset(instack, 0, cpp_sizeof((instack)));
  scc = cpp_assign(Index, "=", cpp_assign(top, "=", 0));
  {
    var i = 1;
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

func dfs(pos: dynamic)
{
  if ((~dp[pos]))
  {
    return dp[pos];
  }
  dp[pos] = 0;
  for (var e in G_new[pos])
  {
    var v = e.first;
    var w = e.second;
    var temp = (dfs(v) + w);
    if ((dp[pos] < temp))
    {
      dp[pos] = temp;
    }
  }
  return cpp_assign(dp[pos], "+=", cost[pos]);
}

func init()
{
  val[0] = 0;
  var temp = 0;
  {
    var i = 1;
    while ((i < maxn))
    {
      temp += i;
      val[i] = (val[(i - 1)] + temp);
      i += 1;
    }
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var s: dynamic;
  init();
  while ((~scanf("%d%d", (&n), (&m))))
  {
    {
      var i = 1;
      while ((i <= n))
      {
        G[i].clear();
        G_new[i].clear();
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < m))
      {
        var u: dynamic;
        var v: dynamic;
        var w: dynamic;
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
      var i = 0;
      while ((i < m))
      {
        var u: dynamic;
        var v: dynamic;
        var w: dynamic;
        u = maps[belong[edges[i].u]];
        v = maps[belong[edges[i].v]];
        w = edges[i].w;
        if ((u != v))
        {
          G_new[u].push_back(make_pair(v, w));
        } else
        {
          var l = 0;
          var r = w;
          var pos: dynamic;
          while ((l <= r))
          {
            var mid = (((l + r)) >> 1);
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
