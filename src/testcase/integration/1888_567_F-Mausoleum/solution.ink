// Translated from solution.cpp.

var dp: dynamic;

var k: dynamic;

var n: dynamic;

var m: dynamic;

func init()
{
  dp.assign(((2 * n) + 1), vector(((2 * n) + 1), -1));
  k.assign(((2 * n) + 1), vector(((2 * n) + 1), vector(5, 0)));
}

func in_cpp()
{
  read(n, m);
  init();
  {
    var x = 0;
    while ((x < m))
    {
      var u: dynamic;
      var v: dynamic;
      var kk: dynamic;
      read(u, kk, v);
      if ((kk[0] == cpp_char("=")))
      {
        k[u][v][0] = cpp_assign(k[v][u][0], "=", 1);
      } else if ((kk[0] == cpp_char(">")))
      {
        if ((kk[1] == cpp_char("=")))
        {
          k[u][v][3] = cpp_assign(k[v][u][4], "=", 1);
        } else
        {
          k[u][v][1] = cpp_assign(k[v][u][2], "=", 1);
        }
      } else
      {
        if ((kk[1] == cpp_char("=")))
        {
          k[u][v][4] = cpp_assign(k[v][u][3], "=", 1);
        } else
        {
          k[u][v][2] = cpp_assign(k[v][u][1], "=", 1);
        }
      }
      x += 1;
    }
  }
}

func check(l: dynamic, r: dynamic, u: dynamic, v: dynamic)
{
  if ((k[u][v][1] || k[u][v][2]))
  {
    return 0;
  }
  {
    var x = 1;
    while ((x <= l))
    {
      if (((k[x][u][0] || k[x][u][1]) || k[x][u][3]))
      {
        return 0;
      }
      if (((k[x][v][0] || k[x][v][1]) || k[x][v][3]))
      {
        return 0;
      }
      x += 1;
    }
  }
  {
    var x = r;
    while ((x <= (2 * n)))
    {
      if (((k[x][u][0] || k[x][u][1]) || k[x][u][3]))
      {
        return 0;
      }
      if (((k[x][v][0] || k[x][v][1]) || k[x][v][3]))
      {
        return 0;
      }
      x += 1;
    }
  }
  return 1;
}

func work(l: dynamic, r: dynamic)
{
  if ((l > r))
  {
    return 0;
  }
  if ((dp[l][r] >= 0))
  {
    return dp[l][r];
  }
  if (((r - l) == 1))
  {
    return cpp_assign(dp[l][r], "=", (check((l - 1), (r + 1), l, r) * 1));
  }
  dp[l][r] = 0;
  if (check((l - 1), (r + 1), l, (l + 1)))
  {
    dp[l][r] += work((l + 2), r);
  }
  if (check((l - 1), (r + 1), l, r))
  {
    dp[l][r] += work((l + 1), (r - 1));
  }
  if (check((l - 1), (r + 1), (r - 1), r))
  {
    dp[l][r] += work(l, (r - 2));
  }
  return dp[l][r];
}

func main()
{
  ios_base.sync_with_stdio(false);
  in_cpp();
  write(work(1, (2 * n)), "\n");
}
