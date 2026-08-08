// Translated from solution.cpp.

var nl = cpp_expression("#inc");

var pb = cpp_expression("#include");

var ll = dynamic;

var VMAX = cpp_expression("#inclu");

var NMAX = cpp_expression("#incl");

var INF = cpp_expression("#include <bits/st");

var f = cpp_construct("pirati.in");

var g = cpp_construct("pirati.out");

var MOD = 1000000007;

var n: dynamic;

var dp = cpp_array(205, 205);

var dist = cpp_array(205, 205);

func lgpow(a: dynamic, b: dynamic)
{
  var ans = 1;
  var baza = a;
  while (b)
  {
    if ((b & 1))
    {
      ans = ((1 * ans) * baza);
      ans %= MOD;
      b -= 1;
    }
    baza = ((1 * baza) * baza);
    baza %= MOD;
    b /= 2;
  }
  return ans;
}

func precalcdp()
{
  {
    var i = 0;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j <= n))
        {
          if (((i == 0) && (j == 0)))
          {
            dp[i][j] = 0;
            j += 1;
            continue;
          }
          if ((i == 0))
          {
            dp[i][j] = 1;
            j += 1;
            continue;
          }
          if ((j == 0))
          {
            dp[i][j] = 0;
            j += 1;
            continue;
          }
          dp[i][j] = (((dp[(i - 1)][j] + dp[i][(j - 1)])) * lgpow(2, (MOD - 2)));
          dp[i][j] = (dp[i][j] % MOD);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func RoyFloyd()
{
  {
    var i = 1;
    while ((i <= n))
    {
      dist[i][i] = 0;
      i += 1;
    }
  }
  {
    var aux = 1;
    while ((aux <= n))
    {
      {
        var i = 1;
        while ((i <= n))
        {
          {
            var j = 1;
            while ((j <= n))
            {
              dist[i][j] = min(dist[i][j], (dist[i][aux] + dist[aux][j]));
              j += 1;
            }
          }
          i += 1;
        }
      }
      aux += 1;
    }
  }
}

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          dist[i][j] = ((2 * n) + 5);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      dist[x][y] = 1;
      dist[y][x] = 1;
      i += 1;
    }
  }
  RoyFloyd();
  precalcdp();
  var ans = 0;
  {
    var lca = 1;
    while ((lca <= n))
    {
      {
        var i = 1;
        while ((i <= n))
        {
          {
            var j = (i + 1);
            while ((j <= n))
            {
              var x = dist[i][lca];
              var y = dist[j][lca];
              var d = ((((x + y) - dist[i][j])) / 2);
              x -= d;
              y -= d;
              ans += dp[y][x];
              ans %= MOD;
              j += 1;
            }
          }
          i += 1;
        }
      }
      lca += 1;
    }
  }
  ans = ((1 * ans) * lgpow(n, (MOD - 2)));
  ans = (ans % MOD);
  write(ans);
}
