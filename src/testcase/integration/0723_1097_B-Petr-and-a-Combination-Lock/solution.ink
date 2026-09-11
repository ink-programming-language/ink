// Translated from solution.cpp.

var N: dynamic = 20;

var a: dynamic = cpp_array(N);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (1 << n)))
    {
      var res: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((((i >> j)) & 1))
          {
            res = (((res + a[j])) % 360);
          } else
          {
            res = ((((res - a[j]) + 360)) % 360);
          }
          j += 1;
        }
      }
      if ((res == 0))
      {
        printf("YES\n");
        return 0;
      }
      i += 1;
    }
  }
  printf("NO\n");
  return 0;
}
