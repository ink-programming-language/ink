// Translated from solution.cpp.

var N: dynamic = 110;

var mod: dynamic = (1e9 + 7);

var a: dynamic = cpp_array(N);

var s: dynamic = cpp_array(110000);

var f: dynamic = cpp_array(110000, 110);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  memset(f, 0, cpp_sizeof((f)));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      f[1][i] =  (((i <= a[1]))) ? 1 : 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      memset(s, 0, cpp_sizeof((s)));
      s[0] = f[(i - 1)][0];
      {
        var j: dynamic = 1;
        while ((j <= k))
        {
          s[j] = (s[(j - 1)] + f[(i - 1)][j]);
          j += 1;
        }
      }
      {
        var j: dynamic = k;
        while ((j >= 0))
        {
          if ((((j - a[i]) - 1) >= 0))
          {
            f[i][j] = (s[j] - s[((j - a[i]) - 1)]);
          } else
          {
            f[i][j] = s[j];
          }
          f[i][j] %= mod;
          j -= 1;
        }
      }
      i += 1;
    }
  }
  write(f[n][k], "\n");
}
