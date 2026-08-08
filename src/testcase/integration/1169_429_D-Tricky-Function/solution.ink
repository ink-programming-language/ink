// Translated from solution.cpp.

func sqr(x: dynamic)
{
  return (x * x);
}

var pi = 3.1415926535897932384626433832795;

var eps = 1e-8;

var N = 100500;

var a = cpp_array(N);

var s = cpp_array(N);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  s[0] = a[0];
  {
    var i = 1;
    while ((i <= cpp_cast(((n - 1)))))
    {
      s[i] = (s[(i - 1)] + a[i]);
      i += 1;
    }
  }
  var ans = (sqr((*min_element((a + 1), (a + n)))) + 1);
  {
    var d = 1;
    while (((d * d) < ans))
    {
      if ((clock() > (CLOCKS_PER_SEC * 1.8)))
      {
        break;
      }
      {
        var i = 0;
        while ((i < cpp_cast(((n - d)))))
        {
          ans = min(ans, (sqr((s[(i + d)] - s[i])) + (d * d)));
          i += 1;
        }
      }
      d += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
