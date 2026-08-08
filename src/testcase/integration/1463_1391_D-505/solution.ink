// Translated from solution.cpp.

var maxn = (1e6 + 7);

var mod = (1e9 + 7);

var MAXN = (1e6 + 7);

var eps = 1e-9;

var INF = 1e18;

var inf = 1e9;

var rnd = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var n: dynamic;

var m: dynamic;

var a = cpp_array(maxn);

var L = ((1 << 4));

var dp = cpp_array(L);

var dp1 = cpp_array(L);

func get(mask: dynamic, col: dynamic)
{
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((((((mask >> i)) & 1)) != ((a[i][col] - cpp_char("0")))))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  return ans;
}

var match_cpp = cpp_array(L, L);

func M(mask: dynamic, mask1: dynamic)
{
  var L: dynamic;
  var R: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      L.push_back((((mask >> i)) & 1));
      R.push_back((((mask1 >> i)) & 1));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      if (((((((L[i] + R[i]) + L[(i - 1)]) + R[(i - 1)])) % 2) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func get1(mask: dynamic, row: dynamic)
{
  var ans = 0;
  {
    var i = 0;
    while ((i < m))
    {
      if ((((((mask >> i)) & 1)) != ((a[row][i] - cpp_char("0")))))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  return ans;
}

func M1(mask: dynamic, mask1: dynamic)
{
  var L: dynamic;
  var R: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      L.push_back((((mask >> i)) & 1));
      R.push_back((((mask1 >> i)) & 1));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < m))
    {
      if (((((((L[i] + R[i]) + L[(i - 1)]) + R[(i - 1)])) % 2) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func print(mask: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      write(((((mask >> i)) & 1)));
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func solve()
{
  read(n, m);
  if (((n == 1) || (m == 1)))
  {
    write("0\n");
    return;
  }
  if (((n >= 4) && (m >= 4)))
  {
    write(-1);
    return;
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  if ((n < 4))
  {
    {
      var i = 0;
      while ((i < ((1 << n))))
      {
        {
          var j = 0;
          while ((j < ((1 << n))))
          {
            match_cpp[i][j] = M(i, j);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < L))
      {
        dp[i] = inf;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < L))
      {
        dp1[i] = inf;
        i += 1;
      }
    }
    {
      var mask = 0;
      while ((mask < ((1 << n))))
      {
        dp[mask] = get(mask, 0);
        mask += 1;
      }
    }
    {
      var i = 1;
      while ((i < m))
      {
        {
          var mask1 = 0;
          while ((mask1 < ((1 << n))))
          {
            var add = get(mask1, i);
            {
              var mask = 0;
              while ((mask < ((1 << n))))
              {
                if (match_cpp[mask][mask1])
                {
                  dp1[mask1] = min(dp1[mask1], (dp[mask] + add));
                }
                mask += 1;
              }
            }
            mask1 += 1;
          }
        }
        {
          var j = 0;
          while ((j < ((1 << n))))
          {
            dp[j] = dp1[j];
            dp1[j] = inf;
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ans = inf;
    {
      var i = 0;
      while ((i < ((1 << n))))
      {
        ans = min(ans, dp[i]);
        i += 1;
      }
    }
    write(ans, cpp_char("\n"));
  } else
  {
    {
      var i = 0;
      while ((i < ((1 << n))))
      {
        {
          var j = 0;
          while ((j < ((1 << n))))
          {
            match_cpp[i][j] = M1(i, j);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < L))
      {
        dp[i] = inf;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < L))
      {
        dp1[i] = inf;
        i += 1;
      }
    }
    {
      var mask = 0;
      while ((mask < ((1 << m))))
      {
        dp[mask] = get1(mask, 0);
        mask += 1;
      }
    }
    {
      var i = 1;
      while ((i < n))
      {
        {
          var mask1 = 0;
          while ((mask1 < ((1 << m))))
          {
            var add = get1(mask1, i);
            {
              var mask = 0;
              while ((mask < ((1 << m))))
              {
                if (match_cpp[mask][mask1])
                {
                  dp1[mask1] = min(dp1[mask1], (dp[mask] + add));
                }
                mask += 1;
              }
            }
            mask1 += 1;
          }
        }
        {
          var j = 0;
          while ((j < ((1 << m))))
          {
            dp[j] = dp1[j];
            dp1[j] = inf;
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ans = inf;
    {
      var i = 0;
      while ((i < ((1 << m))))
      {
        ans = min(ans, dp[i]);
        i += 1;
      }
    }
    write(ans, cpp_char("\n"));
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.precision(20);
  write(fixed);
  var t = 1;
  {
    var i = 0;
    while ((i < t))
    {
      solve();
      i += 1;
    }
  }
  return 0;
}
