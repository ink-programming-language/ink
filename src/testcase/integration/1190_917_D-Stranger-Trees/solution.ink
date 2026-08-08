// Translated from solution.cpp.

func gi()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
  while ((!isdigit(ch)))
  {
    f ^= (ch == cpp_char("-"));
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return if (f) x else (-x);
}

func pow(x: dynamic, y: dynamic)
{
  var ret = 1;
  while (y)
  {
    if ((y & 1))
    {
      ret = (((1 * ret) * x) % 1000000007);
    }
    x = (((1 * x) * x) % 1000000007);
    y >>= 1;
  }
  return ret;
}

var n: dynamic;

var fir = cpp_array(110);

var dis = cpp_array(210);

var nxt = cpp_array(210);

var id: dynamic;

func link(a: dynamic, b: dynamic)
{
  nxt[cpp_update(id, "++")] = fir[a];
  fir[a] = id;
  dis[id] = b;
}

var siz = cpp_array(101);

var dp = cpp_array(101, 101, 101);

var dp = cpp_array(101, 101);

var C = cpp_array(101, 101);

func dfs(x: dynamic, fa: dynamic = -1)
{
  siz[x] = 1;
  dp[x][1][1] = 1;
  {
    var i = fir[x];
    while (i)
    {
      if ((dis[i] == fa))
      {
        i = nxt[i];
        continue;
      }
      dfs(dis[i], x);
      {
        var j = 1;
        while ((j <= siz[x]))
        {
          {
            var k = 1;
            while ((k <= siz[x]))
            {
              dp[j][k] = dp[x][j][k];
              dp[x][j][k] = 0;
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j = 1;
        while ((j <= siz[dis[i]]))
        {
          {
            var k = 1;
            while ((k <= siz[dis[i]]))
            {
              if (dp[dis[i]][j][k])
              {
                {
                  var J = 1;
                  while ((J <= siz[x]))
                  {
                    {
                      var K = 1;
                      while ((K <= siz[x]))
                      {
                        if (dp[J][K])
                        {
                          dp[x][((j + J) - 1)][(k + K)] = (((dp[x][((j + J) - 1)][(k + K)] + ((1 * dp[J][K]) * dp[dis[i]][j][k]))) % 1000000007);
                          dp[x][(j + J)][K] = (((dp[x][(j + J)][K] + ((((1 * dp[J][K]) * dp[dis[i]][j][k]) % 1000000007) * k))) % 1000000007);
                        }
                        K += 1;
                      }
                    }
                    J += 1;
                  }
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      siz[x] += siz[dis[i]];
      i = nxt[i];
    }
  }
}

var ans = cpp_array(101);

func main()
{
  n = gi();
  var a: dynamic;
  var b: dynamic;
  {
    var i = 1;
    while ((i < n))
    {
      a = gi();
      b = gi();
      link(a, b);
      link(b, a);
      i += 1;
    }
  }
  dfs(1);
  ans[(n - 1)] = 1;
  {
    var i = (n - 2);
    var pn = 1;
    while ((~i))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          ans[i] = (((ans[i] + ((1 * dp[1][(n - i)][j]) * j))) % 1000000007);
          j += 1;
        }
      }
      ans[i] = (((1 * ans[i]) * pn) % 1000000007);
      i -= 1;
      pn = (((1 * pn) * n) % 1000000007);
    }
  }
  C[0][0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      C[i][0] = 1;
      {
        var j = 1;
        while ((j <= i))
        {
          C[i][j] = (((C[(i - 1)][(j - 1)] + C[(i - 1)][j])) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (n - 1);
    while ((~i))
    {
      {
        var j = (i + 1);
        while ((j < n))
        {
          ans[i] = ((((ans[i] - (((1 * C[j][i]) * ans[j]) % 1000000007)) + 1000000007)) % 1000000007);
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
