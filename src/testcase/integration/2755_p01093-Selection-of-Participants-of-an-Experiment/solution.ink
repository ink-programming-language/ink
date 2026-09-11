// Translated from solution.cpp.

var a: dynamic = cpp_array(1000);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma(scanf("%d", (&n)), n))
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d", (&a[i]));
        i += 1;
      }
    }
    var ans: dynamic = (1 << 30);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            ans = min(ans, abs((a[i] - a[j])));
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", ans);
  }
}
