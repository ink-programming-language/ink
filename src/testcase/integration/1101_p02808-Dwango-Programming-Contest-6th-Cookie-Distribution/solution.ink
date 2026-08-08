// Translated from solution.cpp.

var N = (1e3 + 2);

var mod = (1e9 + 7);

var dp = cpp_array(N);

var c = cpp_array(N, N);

var ar = cpp_array(N);

var ndp = cpp_array(N);

func add(x: dynamic, y: dynamic)
{
  x += y;
  if ((x >= mod))
  {
    x -= mod;
  }
}

func mul(x: dynamic, y: dynamic)
{
  return ((((1 * x) * y)) % mod);
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var num: dynamic;
  read(n, num);
  {
    i = 1;
    while ((i <= num))
    {
      read(ar[i]);
      i += 1;
    }
  }
  c[0][0] = cpp_assign(dp[0], "=", 1);
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 0;
        while ((j <= i))
        {
          c[j][i] = c[j][(i - 1)];
          if (j)
          {
            add(c[j][i], c[(j - 1)][(i - 1)]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= num))
    {
      {
        k = n;
        while ((k > -1))
        {
          {
            l = min(ar[i], (n - k));
            while ((l > -1))
            {
              add(ndp[(k + l)], mul(mul(dp[k], c[l][(n - k)]), c[(ar[i] - l)][(n - l)]));
              l -= 1;
            }
          }
          k -= 1;
        }
      }
      {
        j = 0;
        while ((j <= n))
        {
          dp[j] = ndp[j];
          ndp[j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[n]);
}
