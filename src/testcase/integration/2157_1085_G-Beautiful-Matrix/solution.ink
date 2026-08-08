// Translated from solution.cpp.

var n: dynamic;

var mat = cpp_array(2021, 2021);

var dp = cpp_array(2021);

var f = cpp_array(2021, 2021);

var fac = cpp_array(2021);

var sum = cpp_array(2021, 2);

var vis = cpp_array(2021);

var vis2 = cpp_array(2021);

func read(x: dynamic)
{
  x = 0;
  var c = getchar();
  {
    while (((c > cpp_char("9")) || (c < cpp_char("0"))))
    {
      c = getchar();
    }
  }
  {
    while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      x = (((x * 10) + c) - cpp_char("0"));
      c = getchar();
    }
  }
}

func mul(x: dynamic, y: dynamic)
{
  var ans = 1;
  {
    while (y)
    {
      if ((y & 1))
      {
        ans = ((ans * x) % 998244353);
      }
      x = ((x * x) % 998244353);
      y >>= 1;
    }
  }
  return ans;
}

func update(x: dynamic, add: dynamic, pos: dynamic)
{
  {
    while ((x <= n))
    {
      sum[pos][x] += add;
      x += (x & ((-x)));
    }
  }
}

func query(x: dynamic, pos: dynamic)
{
  var ans = 0;
  {
    while ((x > 0))
    {
      ans += sum[pos][x];
      x -= (x & ((-x)));
    }
  }
  return ans;
}

func main()
{
  read(n);
  if ((n == 1))
  {
    puts("0");
    return 0;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          read(mat[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[2] = cpp_assign(fac[0], "=", 1);
  {
    var i = 3;
    while ((i <= n))
    {
      dp[i] = ((((i - 1)) * ((dp[(i - 1)] + dp[(i - 2)]))) % 998244353);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      fac[i] = ((fac[(i - 1)] * i) % 998244353);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      f[i][0] = fac[i];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= i))
        {
          f[i][j] = ((((f[i][(j - 1)] - f[(i - 1)][(j - 1)]) + 998244353)) % 998244353);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      update(i, 1, 0);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      update(mat[1][i], -1, 0);
      ans = (((ans + (fac[(n - i)] * query(mat[1][i], 0)))) % 998244353);
      i += 1;
    }
  }
  ans = ((ans * mul(dp[n], (n - 1))) % 998244353);
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    var bk: dynamic;
    var i = 2;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          vis[j] = cpp_assign(vis2[j], "=", 0);
          update(j, 1, 0);
          update(j, 1, 1);
          j += 1;
        }
      }
      var tmp = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          if ((!vis[mat[(i - 1)][j]]))
          {
            vis[mat[(i - 1)][j]] = 1;
            update(mat[(i - 1)][j], -1, 1);
          }
          bk = (!vis[mat[i][j]]);
          if (bk)
          {
            vis[mat[i][j]] = 1;
            update(mat[i][j], -1, 1);
          }
          update(mat[i][j], -1, 0);
          vis2[mat[i][j]] = 1;
          a = query(mat[i][j], 0);
          b = query(mat[i][j], 1);
          c = (query(n, 1) - query(mat[i][j], 1));
          a -= b;
          tmp = ((((tmp + ((f[(n - j)][(((b - 1) + bk) + c)] * b) % 998244353)) + ((f[(n - j)][((b + bk) + c)] * a) % 998244353))) % 998244353);
          if (((mat[i][j] > mat[(i - 1)][j]) && (!vis2[mat[(i - 1)][j]])))
          {
            tmp = ((((tmp - f[(n - j)][((b + bk) + c)]) + 998244353)) % 998244353);
          }
          j += 1;
        }
      }
      ans = (((ans + (tmp * mul(dp[n], (n - i))))) % 998244353);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
