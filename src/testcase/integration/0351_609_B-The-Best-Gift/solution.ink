// Translated from solution.cpp.

var a = cpp_array(15);

func main()
{
  var n: dynamic;
  var m: dynamic;
  scanf("%d%d", (&n), (&m));
  memset(a, 0, cpp_sizeof(a));
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      scanf("%d", (&x));
      a[x] += 1;
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= m))
    {
      {
        var j = (i + 1);
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
