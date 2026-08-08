// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var W: dynamic;
  read(n, W);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i], w[i], m[i]);
      i += 1;
    }
  }
  var dp = cpp_construct((W + 1), 0);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var k = 0;
        while ((m[i] > 0))
        {
          var key = min(m[i], cpp_cast(((1 << k))));
          m[i] -= key;
          {
            var j = W;
            while ((j >= 0))
            {
              if (((j + (key * w[i])) <= W))
              {
                dp[(j + (key * w[i]))] = max(dp[(j + (key * w[i]))], (dp[j] + (key * v[i])));
              }
              j -= 1;
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var j = 0;
    while ((j <= W))
    {
      ans = max(ans, dp[j]);
      j += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
