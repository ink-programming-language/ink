// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var ls = cpp_array(20);

var pref = cpp_array(20);

var y: dynamic;

var dp = cpp_array(100000);

func count()
{
  fill(dp, (dp + ((1 << n))), 0);
  dp[0] = 1;
  {
    var mask = 0;
    while ((mask < ((1 << n))))
    {
      if ((dp[mask] == 0))
      {
        mask += 1;
        continue;
      }
      var cnt = 0;
      var tmp = mask;
      while ((tmp > 0))
      {
        if ((tmp & (1 == 1)))
        {
          cnt += 1;
        }
        tmp /= 2;
      }
      {
        var i = 0;
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

func main()
{
  read(n, y, m);
  y -= 2000;
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      ls[(u - 1)] |= (1 << ((v - 1)));
      i += 1;
    }
  }
  fill(pref, (pref + n), -1);
  {
    var i = 0;
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
          var tmp = count();
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
    var i = 0;
    while ((i < n))
    {
      write((pref[i] + 1), " ");
      i += 1;
    }
  }
  write("\n");
  return 0;
}
