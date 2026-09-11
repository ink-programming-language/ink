// Translated from solution.cpp.

var nl: dynamic = cpp_expression("#inc");

var pb: dynamic = cpp_expression("#include");

var ll: dynamic = dynamic;

var VMAX: dynamic = cpp_expression("#inclu");

var NMAX: dynamic = cpp_expression("#incl");

var INF: dynamic = cpp_expression("#include <bits/st");

var f: dynamic = cpp_construct("pirati.in");

var g: dynamic = cpp_construct("pirati.out");

var MOD: dynamic = 1000000007;

var n: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(205, 205);

var dist: dynamic = cpp_array(205, 205);

func lgpow(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  var baza: dynamic = a;
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

func precalcdp() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
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

func RoyFloyd() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      dist[i][i] = 0;
      i += 1;
    }
  }
  {
    var aux: dynamic = 1;
    while ((aux <= n))
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          {
            var j: dynamic = 1;
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

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      dist[x][y] = 1;
      dist[y][x] = 1;
      i += 1;
    }
  }
  RoyFloyd();
  precalcdp();
  var ans: dynamic = 0;
  {
    var lca: dynamic = 1;
    while ((lca <= n))
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          {
            var j: dynamic = (i + 1);
            while ((j <= n))
            {
              var x: dynamic = dist[i][lca];
              var y: dynamic = dist[j][lca];
              var d: dynamic = ((((x + y) - dist[i][j])) / 2);
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
