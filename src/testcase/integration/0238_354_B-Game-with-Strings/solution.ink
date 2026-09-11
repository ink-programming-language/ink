// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var N: dynamic = (1e5 + 10);

var inf: dynamic = 110101010;

var a: dynamic = cpp_array(25, 25);

var dp: dynamic = cpp_array((1 << 20), 40);

var n: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array((1 << 20), 40);

func dfs(x: dynamic, st: dynamic) -> dynamic
{
  if (vis[x][st])
  {
    return dp[x][st];
  }
  var res: dynamic = dp[x][st];
  vis[x][st] = 1;
  if ((x == ((n * 2) - 2)))
  {
    res = 0;
  } else
  {
    if ((x & 1))
    {
      res = (-inf);
    } else
    {
      res = inf;
    }
    var mask: dynamic = cpp_array(30);
    memset(mask, 0, cpp_sizeof((mask)));
    var cnt: dynamic = 0;
    {
      var j: dynamic = 0;
      while ((j <= (x + 1)))
      {
        var first: dynamic = ((x + 1) - j);
        var second: dynamic = j;
        if (((first >= n) || (second >= n)))
        {
          j += 1;
          continue;
        }
        mask[(a[first][second] - cpp_char("a"))] |= ((1 << cnt));
        cnt += 1;
        j += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i <= 25))
      {
        if (mask[i])
        {
          var xt: dynamic = cpp_uninitialized();
          if (((x + 1) < n))
          {
            xt = (((st | ((st << 1)))) & mask[i]);
          } else
          {
            xt = (((st | ((st >> 1)))) & mask[i]);
          }
          if ((!xt))
          {
            i += 1;
            continue;
          }
          var tmp: dynamic = 0;
          if ((!i))
          {
            tmp = 1;
          } else if ((i == 1))
          {
            tmp = -1;
          }
          if ((x & 1))
          {
            res = max(res, (tmp + dfs((x + 1), xt)));
          } else
          {
            res = min(res, (tmp + dfs((x + 1), xt)));
          }
        }
        i += 1;
      }
    }
  }
  if ((!x))
  {
    if ((a[0][0] == cpp_char("a")))
    {
      res += 1;
    } else if ((a[0][0] == cpp_char("b")))
    {
      res -= 1;
    }
  }
  return res;
}

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      scanf("%s", a[i]);
      i += 1;
    }
  }
  var res: dynamic = dfs(0, 1);
  if ((!res))
  {
    printf("DRAW\n");
  } else if ((res > 0))
  {
    printf("FIRST\n");
  } else
  {
    printf("SECOND\n");
  }
  return 0;
}
