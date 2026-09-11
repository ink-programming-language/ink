// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ls: dynamic = cpp_array(20);

var pref: dynamic = cpp_array(20);

var y: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(100000);

func count() -> dynamic
{
  fill(dp, (dp + ((1 << n))), 0);
  dp[0] = 1;
  {
    var mask: dynamic = 0;
    while ((mask < ((1 << n))))
    {
      if ((dp[mask] == 0))
      {
        mask += 1;
        continue;
      }
      var cnt: dynamic = 0;
      var tmp: dynamic = mask;
      while ((tmp > 0))
      {
        if ((tmp & (1 == 1)))
        {
          cnt += 1;
        }
        tmp /= 2;
      }
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((((((pref[i] == -1) || (pref[i] == ((n - cnt) - 1)))) && ((((ls[i] & mask)) == ls[i]))) && ((((mask & ((1 << i)))) == 0))))
          {
            dp[(mask | ((1 << i)))] += dp[mask];
          }
          i += 1;
        }
      }
      mask += 1;
    }
  }
  return dp[(((1 << n)) - 1)];
}

func main() -> dynamic
{
  read(n, y, m);
  y -= 2000;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      ls[(u - 1)] |= (1 << ((v - 1)));
      i += 1;
    }
  }
  fill(pref, (pref + n), -1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        while (true)
        {
          pref[i] += 1;
          if ((pref[i] == n))
          {
            write("The times have changed", "\n");
            return 0;
          }
          var tmp: dynamic = count();
          if ((tmp < y))
          {
            y -= tmp;
          } else
          {
            break;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write((pref[i] + 1), " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
