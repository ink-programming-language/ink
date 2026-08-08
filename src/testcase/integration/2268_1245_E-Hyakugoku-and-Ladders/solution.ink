// Translated from solution.cpp.

var a = cpp_array(10, 10);

var dp = cpp_array(10, 10);

var vis = cpp_array(10, 10);

func solve(i: dynamic, j: dynamic)
{
  if (((i == 9) && (j == 9)))
  {
    return 0;
  }
  if (vis[i][j])
  {
    return dp[i][j];
  }
  var cord = ((i * 10) + j);
  vis[i][j] = 1;
  var cur = 0;
  var cnt = 0;
  {
    var k = 1;
    while ((k < 7))
    {
      var x = (((cord + k)) / 10);
      var y = (((cord + k)) % 10);
      if ((x <= 9))
      {
        if (a[x][y])
        {
          var t = (a[x][y] % 2);
          if (t)
          {
            cur = (cur + (((min(solve(x, y), solve((x + a[x][y]), (9 - y))) + 1.0)) / 6.0));
          } else
          {
            cur = (cur + (((min(solve(x, y), solve((x + a[x][y]), y)) + 1.0)) / 6.0));
          }
        } else
        {
          cur = (cur + (((solve(x, y) + 1.0)) / 6.0));
        }
      } else
      {
        cnt += 1;
      }
      k += 1;
    }
  }
  cur = (((((cur + ((((cnt * 1.0)) / 6.0)))) * 6.0)) / ((6.0 - cnt)));
  return cpp_assign(dp[i][j], "=", cur);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var TESTS = 1;
  while (cpp_update(TESTS, "--"))
  {
    {
      var i = 0;
      while ((i < 10))
      {
        {
          var j = 0;
          while ((j < 10))
          {
            read(a[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    reverse(a, (a + 10));
    {
      var i = 1;
      while ((i < 10))
      {
        reverse(a[i], (a[i] + 10));
        i += 1;
        i += 1;
      }
    }
    var cur = 0;
    write(setprecision(14));
    write(fixed, solve(0, 0));
  }
  return 0;
}
