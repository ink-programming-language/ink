// Translated from solution.cpp.

var N: dynamic = (1e3 + 2);

var mod: dynamic = (1e9 + 7);

var dp: dynamic = cpp_array(N);

var c: dynamic = cpp_array(N, N);

var ar: dynamic = cpp_array(N);

var ndp: dynamic = cpp_array(N);

func add(x: dynamic, y: dynamic) -> dynamic
{
  x += y;
  if ((x >= mod))
  {
    x -= mod;
  }
}

func mul(x: dynamic, y: dynamic) -> dynamic
{
  return ((((1 * x) * y)) % mod);
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_uninitialized();
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
