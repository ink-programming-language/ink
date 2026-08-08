// Translated from solution.cpp.

var INF = 3000000000000000000;

var dp = cpp_array(3, 1000001);

func pf(a: dynamic, s: dynamic)
{
  var i: dynamic;
  {
    i = 2;
    while (((i * i) <= a))
    {
      if (((a % i) == 0))
      {
        s.insert(i);
        while (((a % i) == 0))
        {
          a /= i;
        }
      }
      i += 1;
    }
  }
  if ((a > 1))
  {
    s.insert(a);
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var i: dynamic;
  var j: dynamic;
  var n: dynamic;
  var A: dynamic;
  var B: dynamic;
  read(n, A, B);
  {
    i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var p: dynamic;
  pf((a[0] - 1), p);
  pf(a[0], p);
  pf((a[0] + 1), p);
  pf((a[(n - 1)] - 1), p);
  pf(a[(n - 1)], p);
  pf((a[(n - 1)] + 1), p);
  var ans = INF;
  {
    typeof(p.begin()) = p.begin();
    while ((it != p.end()))
    {
      var d = (*it);
      {
        i = 0;
        while ((i < 3))
        {
          dp[0][i] = 0;
          i += 1;
        }
      }
      {
        i = 0;
        while ((i < n))
        {
          var x = (a[i] % d);
          if ((x == 0))
          {
            dp[(i + 1)][0] = dp[i][0];
            dp[(i + 1)][1] = min(dp[i][0], (dp[i][1] + A));
            dp[(i + 1)][2] = min(dp[i][1], dp[i][2]);
          } else if (((x == 1) || (x == (d - 1))))
          {
            dp[(i + 1)][0] = (dp[i][0] + B);
            dp[(i + 1)][1] = min((dp[i][0] + B), (dp[i][1] + A));
            dp[(i + 1)][2] = min((dp[i][1] + B), (dp[i][2] + B));
          } else
          {
            dp[(i + 1)][0] = INF;
            dp[(i + 1)][1] = (min(dp[i][0], dp[i][1]) + A);
            dp[(i + 1)][2] = dp[(i + 1)][1];
          }
          i += 1;
        }
      }
      var tmp = INF;
      {
        i = 0;
        while ((i < 3))
        {
          tmp = min(tmp, dp[n][i]);
          i += 1;
        }
      }
      ans = min(ans, tmp);
      it += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
