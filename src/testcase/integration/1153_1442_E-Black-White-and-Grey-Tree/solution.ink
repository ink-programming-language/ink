// Translated from solution.cpp.

func mini(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func maxi(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

var N = (2e5 + 5);

var oo = 1e9;

var adj = cpp_array(N);

var dp = cpp_array(2, N);

var a = cpp_array(N);

var n: dynamic;

var ans: dynamic;

func dfs(u: dynamic, p: dynamic = -1)
{
  for (var v in adj[u])
  {
    if ((v != p))
    {
      dfs(v, u);
    }
  }
  dp[u][0] = cpp_assign(dp[u][1], "=", oo);
  if ((a[u] == 0))
  {
    var d = oo;
    {
      var val = 0;
      while ((val < 2))
      {
        dp[u][val] = 0;
        var tmp = make_pair(0, 0);
        for (var v in adj[u])
        {
          if ((v != p))
          {
            var res = min((dp[v][0] + val), (dp[v][1] + ((!val))));
            maxi(dp[u][val], res);
            maxi(tmp.second, res);
            if ((tmp.second > tmp.first))
            {
              swap(tmp.second, tmp.first);
            }
          }
        }
        mini(d, (tmp.first + tmp.second));
        val += 1;
      }
    }
    maxi(ans, d);
  } else
  {
    var val = (a[u] - 1);
    dp[u][val] = 0;
    var tmp = make_pair(0, 0);
    for (var v in adj[u])
    {
      if ((v != p))
      {
        var res = min((dp[v][0] + val), (dp[v][1] + ((!val))));
        maxi(dp[u][val], res);
        maxi(tmp.second, res);
        if ((tmp.second > tmp.first))
        {
          swap(tmp.second, tmp.first);
        }
      }
    }
    maxi(ans, (tmp.first + tmp.second));
  }
}

func solve()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      adj[i].clear();
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  ans = 0;
  dfs(1);
  write((((((ans + 1)) >> 1)) + 1), "\n");
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
