// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 7);

var mod: dynamic = (1e9 + 7);

var MAXN: dynamic = (1e6 + 7);

var eps: dynamic = 1e-9;

var INF: dynamic = 1e18;

var inf: dynamic = 1e9;

var rnd: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var L: dynamic = ((1 << 4));

var dp: dynamic = cpp_array(L);

var dp1: dynamic = cpp_array(L);

func get(mask: dynamic, col: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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

var match_cpp: dynamic = cpp_array(L, L);

func M(mask: dynamic, mask1: dynamic) -> dynamic
{
  var L: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      L.push_back((((mask >> i)) & 1));
      R.push_back((((mask1 >> i)) & 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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

func get1(mask: dynamic, row: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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

func M1(mask: dynamic, mask1: dynamic) -> dynamic
{
  var L: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      L.push_back((((mask >> i)) & 1));
      R.push_back((((mask1 >> i)) & 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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

func print(mask: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(((((mask >> i)) & 1)));
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func solve() -> dynamic
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
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  if ((n < 4))
  {
    {
      var i: dynamic = 0;
      while ((i < ((1 << n))))
      {
        {
          var j: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < L))
      {
        dp[i] = inf;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < L))
      {
        dp1[i] = inf;
        i += 1;
      }
    }
    {
      var mask: dynamic = 0;
      while ((mask < ((1 << n))))
      {
        dp[mask] = get(mask, 0);
        mask += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < m))
      {
        {
          var mask1: dynamic = 0;
          while ((mask1 < ((1 << n))))
          {
            var add: dynamic = get(mask1, i);
            {
              var mask: dynamic = 0;
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
          var j: dynamic = 0;
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
    var ans: dynamic = inf;
    {
      var i: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < ((1 << n))))
      {
        {
          var j: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < L))
      {
        dp[i] = inf;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < L))
      {
        dp1[i] = inf;
        i += 1;
      }
    }
    {
      var mask: dynamic = 0;
      while ((mask < ((1 << m))))
      {
        dp[mask] = get1(mask, 0);
        mask += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        {
          var mask1: dynamic = 0;
          while ((mask1 < ((1 << m))))
          {
            var add: dynamic = get1(mask1, i);
            {
              var mask: dynamic = 0;
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
          var j: dynamic = 0;
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
    var ans: dynamic = inf;
    {
      var i: dynamic = 0;
      while ((i < ((1 << m))))
      {
        ans = min(ans, dp[i]);
        i += 1;
      }
    }
    write(ans, cpp_char("\n"));
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.precision(20);
  write(fixed);
  var t: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      solve();
      i += 1;
    }
  }
  return 0;
}
