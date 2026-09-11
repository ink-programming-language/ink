// Translated from solution.cpp.

func mini(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func maxi(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

var N: dynamic = (2e5 + 5);

var oo: dynamic = 1e9;

var adj: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(2, N);

var a: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func dfs(u: dynamic, p: dynamic = -1) -> dynamic
{
  for (var v: dynamic in adj[u])
  {
    if ((v != p))
    {
      dfs(v, u);
    }
  }
  dp[u][0] = cpp_assign(dp[u][1], "=", oo);
  if ((a[u] == 0))
  {
    var d: dynamic = oo;
    {
      var val: dynamic = 0;
      while ((val < 2))
      {
        dp[u][val] = 0;
        var tmp: dynamic = make_pair(0, 0);
        for (var v: dynamic in adj[u])
        {
          if ((v != p))
          {
            var res: dynamic = min((dp[v][0] + val), (dp[v][1] + ((!val))));
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
    var val: dynamic = (a[u] - 1);
    dp[u][val] = 0;
    var tmp: dynamic = make_pair(0, 0);
    for (var v: dynamic in adj[u])
    {
      if ((v != p))
      {
        var res: dynamic = min((dp[v][0] + val), (dp[v][1] + ((!val))));
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

func solve() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      adj[i].clear();
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
