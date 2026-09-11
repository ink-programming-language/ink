// Translated from solution.cpp.

var maxn: dynamic = (1e5 + 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array((3 * maxn));

var cir: dynamic = cpp_array((3 * maxn));

var suff: dynamic = cpp_array((3 * maxn));

var G: dynamic = cpp_array((3 * maxn));

var path: dynamic = cpp_uninitialized();

func dfs(now: dynamic, pre: dynamic) -> dynamic
{
  path.push(now);
  vis[now] = 1;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(G[now].size())))
    {
      var Next: dynamic = G[now][i];
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
        var maxc: dynamic = now;
        var minc: dynamic = now;
        while ((!path.empty()))
        {
          var temp: dynamic = path.top();
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

func main() -> dynamic
{
  scanf("%lld %lld", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%lld %lld", (&u), (&v));
      G[u].push_back(v);
      G[v].push_back(u);
      i += 1;
    }
  }
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i: dynamic = 0;
    while ((i <= (n + 1)))
    {
      cir[i] = (n + 1);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = n;
    while ((i >= 1))
    {
      cir[i] = min(cir[i], cir[(i + 1)]);
      i -= 1;
    }
  }
  {
    var i: dynamic = n;
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
    var L: dynamic = l;
    var R: dynamic = r;
    var p: dynamic = cpp_uninitialized();
    while ((L <= R))
    {
      var m: dynamic = (((L + R)) / 2);
      if ((cir[m] <= r))
      {
        L = (m + 1);
      } else
      {
        p = m;
        R = (m - 1);
      }
    }
    var ans: dynamic = ((suff[l] - suff[p]) + (((((r - p) + 1)) * (((r - p) + 2))) / 2));
    printf("%lld\n", ans);
  }
  return 0;
}
