// Translated from solution.cpp.

var maxn = (1e5 + 10);

var n: dynamic;

var m: dynamic;

var q: dynamic;

var l: dynamic;

var r: dynamic;

var vis = cpp_array((3 * maxn));

var cir = cpp_array((3 * maxn));

var suff = cpp_array((3 * maxn));

var G = cpp_array((3 * maxn));

var path: dynamic;

func dfs(now: dynamic, pre: dynamic)
{
  path.push(now);
  vis[now] = 1;
  {
    var i = 0;
    while ((i < cpp_cast(G[now].size())))
    {
      var Next = G[now][i];
      if ((Next == pre))
      {
        i += 1;
        continue;
      }
      if ((!vis[Next]))
      {
        dfs(Next, now);
      }
      if ((vis[Next] == 1))
      {
        var maxc = now;
        var minc = now;
        while ((!path.empty()))
        {
          var temp = path.top();
          path.pop();
          maxc = max(maxc, temp);
          minc = min(minc, temp);
          if ((temp == Next))
          {
            break;
          }
        }
        cir[minc] = maxc;
      }
      i += 1;
    }
  }
  if (((!path.empty()) && (path.top() == now)))
  {
    path.pop();
  }
  vis[now] = 2;
}

func main()
{
  scanf("%lld %lld", (&n), (&m));
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%lld %lld", (&u), (&v));
      G[u].push_back(v);
      G[v].push_back(u);
      i += 1;
    }
  }
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i = 0;
    while ((i <= (n + 1)))
    {
      cir[i] = (n + 1);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        dfs(i, 0);
      }
      i += 1;
    }
  }
  suff[n] = cpp_assign(suff[(n + 1)], "=", 0);
  {
    var i = n;
    while ((i >= 1))
    {
      cir[i] = min(cir[i], cir[(i + 1)]);
      i -= 1;
    }
  }
  {
    var i = n;
    while ((i >= 1))
    {
      suff[i] = ((cir[i] - i) + suff[(i + 1)]);
      i -= 1;
    }
  }
  scanf("%lld", (&q));
  while (cpp_update(q, "--"))
  {
    scanf("%lld %lld", (&l), (&r));
    var L = l;
    var R = r;
    var p: dynamic;
    while ((L <= R))
    {
      var m = (((L + R)) / 2);
      if ((cir[m] <= r))
      {
        L = (m + 1);
      } else
      {
        p = m;
        R = (m - 1);
      }
    }
    var ans = ((suff[l] - suff[p]) + (((((r - p) + 1)) * (((r - p) + 2))) / 2));
    printf("%lld\n", ans);
  }
  return 0;
}
