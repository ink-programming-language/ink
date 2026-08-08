// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var a = cpp_array(55);

var b = cpp_array(55);

var x: dynamic;

var f = cpp_array(5050, 55);

var c = cpp_array(55);

var now: dynamic;

var res: dynamic;

func doit()
{
  {
    var i = 0;
    while ((i <= m))
    {
      f[(n + 1)][i] = 0;
      i += 1;
    }
  }
  {
    var i = n;
    while ((i > 0))
    {
      {
        var j = 0;
        while ((j <= m))
        {
          f[i][j] = (c[i] * ((a[i] + (if (((j + a[i]) > m)) now else min(f[(i + 1)][(j + a[i])], now)))));
          f[i][j] += (((1 - c[i])) * ((b[i] + (if (((j + b[i]) > m)) now else min(f[(i + 1)][(j + b[i])], now)))));
          j += 1;
        }
      }
      i -= 1;
    }
  }
  res = f[1][0];
}

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d%d", (a + i), (b + i), (&x));
      c[i] = (x / 100.0);
      i += 1;
    }
  }
  var l = 0.0;
  var r = 1000000000.0;
  {
    var i = 1;
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
