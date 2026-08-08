// Translated from solution.cpp.

var N = 20;

var a = cpp_array(N);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (1 << n)))
    {
      var res = 0;
      {
        var j = 0;
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
