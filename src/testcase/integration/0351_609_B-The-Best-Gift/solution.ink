// Translated from solution.cpp.

var a: dynamic = cpp_array(15);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  memset(a, 0, cpp_sizeof(a));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      a[x] += 1;
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      {
        var j: dynamic = (i + 1);
        while ((j <= m))
        {
          ans = (ans + (a[i] * a[j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
