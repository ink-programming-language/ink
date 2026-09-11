// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var W: dynamic = cpp_uninitialized();
  read(n, W);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i], w[i], m[i]);
      i += 1;
    }
  }
  var dp: dynamic = cpp_construct((W + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var k: dynamic = 0;
        while ((m[i] > 0))
        {
          var key: dynamic = min(m[i], cpp_cast(((1 << k))));
          m[i] -= key;
          {
            var j: dynamic = W;
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
  var ans: dynamic = 0;
  {
    var j: dynamic = 0;
    while ((j <= W))
    {
      ans = max(ans, dp[j]);
      j += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
