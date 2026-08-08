// Translated from solution.cpp.

var N = 110;

var mod = (1e9 + 7);

var a = cpp_array(N);

var s = cpp_array(110000);

var f = cpp_array(110000, 110);

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  memset(f, 0, cpp_sizeof((f)));
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= k))
    {
      f[1][i] = if (((i <= a[1]))) 1 else 0;
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= n))
    {
      memset(s, 0, cpp_sizeof((s)));
      s[0] = f[(i - 1)][0];
      {
        var j = 1;
        while ((j <= k))
        {
          s[j] = (s[(j - 1)] + f[(i - 1)][j]);
          j += 1;
        }
      }
      {
        var j = k;
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
