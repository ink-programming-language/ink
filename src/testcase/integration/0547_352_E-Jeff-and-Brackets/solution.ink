// Translated from solution.cpp.

var N: dynamic = (2e5 + 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(44, 22);

class uzi
{
  var A: dynamic = cpp_array(44, 44);
  func uzi() -> dynamic
  {
      memset(A, 0x3f3f3f, cpp_sizeof(A));
    }
}

var G: dynamic = cpp_uninitialized();

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i <= 40))
    {
      {
        var j: dynamic = 0;
        while ((j <= 40))
        {
          {
            var k: dynamic = 0;
            while ((k <= 40))
            {
              c.A[i][j] = min((a.A[i][k] + b.A[k][j]), c.A[i][j]);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return c;
}

func pm() -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i <= 40))
    {
      c.A[i][i] = 0;
      i += 1;
    }
  }
  while (m)
  {
    if ((m & 1))
    {
      c = (c * G);
    }
    G = (G * G);
    m >>= 1;
  }
  return c;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= 40))
    {
      {
        var j: dynamic = 0;
        while ((j <= n))
        {
          {
            var k: dynamic = 0;
            while ((k <= 40))
            {
              dp[j][k] = 1e9;
              if ((!j))
              {
                if ((k == i))
                {
                  dp[j][k] = 0;
                }
              } else
              {
                if (k)
                {
                  dp[j][k] = min(dp[j][k], (dp[(j - 1)][(k - 1)] + a[(j - 1)]));
                }
                if (((k + 1) <= 40))
                {
                  dp[j][k] = min(dp[j][k], (dp[(j - 1)][(k + 1)] + b[(j - 1)]));
                }
              }
              k += 1;
            }
          }
          {
            var k: dynamic = 0;
            while ((k <= 40))
            {
              G.A[i][k] = dp[n][k];
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(pm().A[0][0]);
  return 0;
}
