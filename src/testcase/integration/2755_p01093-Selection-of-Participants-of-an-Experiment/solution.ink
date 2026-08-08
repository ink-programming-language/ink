// Translated from solution.cpp.

var a = cpp_array(1000);

func main()
{
  var n: dynamic;
  while (cpp_comma(scanf("%d", (&n)), n))
  {
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%d", (&a[i]));
        i += 1;
      }
    }
    var ans = (1 << 30);
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = (i + 1);
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
