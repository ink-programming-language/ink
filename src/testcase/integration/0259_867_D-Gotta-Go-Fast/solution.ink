// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(55);

var b: dynamic = cpp_array(55);

var x: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(5050, 55);

var c: dynamic = cpp_array(55);

var now: dynamic = cpp_uninitialized();

var res: dynamic = cpp_uninitialized();

func doit() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= m))
    {
      f[(n + 1)][i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i > 0))
    {
      {
        var j: dynamic = 0;
        while ((j <= m))
        {
          f[i][j] = (c[i] * ((a[i] + ( (((j + a[i]) > m)) ? now : min(f[(i + 1)][(j + a[i])], now)))));
          f[i][j] += (((1 - c[i])) * ((b[i] + ( (((j + b[i]) > m)) ? now : min(f[(i + 1)][(j + b[i])], now)))));
          j += 1;
        }
      }
      i -= 1;
    }
  }
  res = f[1][0];
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d%d%d", (a + i), (b + i), (&x));
      c[i] = (x / 100.0);
      i += 1;
    }
  }
  var l: dynamic = 0.0;
  var r: dynamic = 1000000000.0;
  {
    var i: dynamic = 1;
    while ((i <= 233))
    {
      now = (((l + r)) / 2.0);
      doit();
      if ((res > now))
      {
        l = now;
      } else
      {
        r = now;
      }
      i += 1;
    }
  }
  printf("%.233lf", cpp_cast(l));
}
