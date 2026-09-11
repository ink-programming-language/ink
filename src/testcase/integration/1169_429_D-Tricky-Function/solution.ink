// Translated from solution.cpp.

func sqr(x: dynamic) -> dynamic
{
  return (x * x);
}

var pi: dynamic = 3.1415926535897932384626433832795;

var eps: dynamic = 1e-8;

var N: dynamic = 100500;

var a: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((n))))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  s[0] = a[0];
  {
    var i: dynamic = 1;
    while ((i <= cpp_cast(((n - 1)))))
    {
      s[i] = (s[(i - 1)] + a[i]);
      i += 1;
    }
  }
  var ans: dynamic = (sqr((*min_element((a + 1), (a + n)))) + 1);
  {
    var d: dynamic = 1;
    while (((d * d) < ans))
    {
      if ((clock() > (CLOCKS_PER_SEC * 1.8)))
      {
        break;
      }
      {
        var i: dynamic = 0;
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
