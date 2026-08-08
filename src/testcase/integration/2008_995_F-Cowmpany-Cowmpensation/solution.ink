// Translated from solution.cpp.

var n: dynamic;

var d: dynamic;

var dp = cpp_array(3006, 3006);

var c = cpp_array(3006, 3006);

var tmp = 1;

var ans = 0;

var mod = (1e9 + 7);

var e = cpp_array(3006);

func dfs(u: dynamic, p: dynamic = 0)
{
  {
    var i = 1;
    while ((i <= n))
    {
      dp[u][i] = 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < e[u].size()))
    {
      var v = e[u][i];
      dfs(v, u);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j < e[u].size()))
        {
          var v = e[u][j];
          if ((v == p))
          {
            j += 1;
            continue;
          }
          dp[u][i] = (((dp[u][i] * dp[v][i])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      dp[u][i] = (((dp[u][i] + dp[u][(i - 1)])) % mod);
      i += 1;
    }
  }
  return 0;
}

func init_com()
{
  {
    var i = 0;
    while ((i < 3006))
    {
      c[i][0] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < 3006))
    {
      {
        var j = 1;
        while ((j <= i))
        {
          c[i][j] = (((c[(i - 1)][j] + c[(i - 1)][(j - 1)])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}

func fast_pow(x: dynamic, y: dynamic)
{
  if ((y == 1))
  {
    return x;
  }
  if ((y == 0))
  {
    return 1;
  }
  var res = fast_pow(x, (y / 2));
  if ((y % 2))
  {
    return ((((((res * res)) % mod)) * x) % mod);
  }
  return ((res * res) % mod);
}

func inv(x: dynamic)
{
  return fast_pow(x, (mod - 2));
}

func main()
{
  init_com();
  read(n, d);
  {
    var i = 2;
    while ((i <= n))
    {
      var x: dynamic;
      read(x);
      e[x].push_back(i);
      i += 1;
    }
  }
  memset(dp, 0, cpp_sizeof((dp)));
  dfs(1);
  if ((d <= n))
  {
    return cpp_comma((cout << dp[1][d]), 0);
  }
  {
    var i = 1;
    while ((i <= min(n, d)))
    {
      tmp = (((tmp * (((d - i) + 1)))) % mod);
      tmp = (((tmp * inv(i))) % mod);
      {
        var j = (i - 1);
        while ((j >= 1))
        {
          var dec = c[i][j];
          dec = (((dec * dp[1][j])) % mod);
          dp[1][i] = ((((dp[1][i] - dec) + mod)) % mod);
          j -= 1;
        }
      }
      ans = (((ans + ((dp[1][i] * tmp) % mod))) % mod);
      i += 1;
    }
  }
  write(ans);
}
