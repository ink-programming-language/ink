// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(300005);

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    var j = 1;
    while ((i <= n))
    {
      while (1)
      {
        if ((((j + 1) > n) || (a[(j + 1)] != (j + 1))))
        {
          break;
        }
        if ((((j + 2) > n) || (a[(j + 2)] == (j + 2))))
        {
          break;
        }
        j += 2;
      }
      {
        var k = i;
        while ((k <= j))
        {
          if (((a[k] < i) || (a[k] > j)))
          {
            return cpp_comma(puts("No"), 0);
          }
          k += 1;
        }
      }
      {
        var k = i;
        var x = 0;
        var y = 0;
        while ((k <= j))
        {
          if ((a[k] < k))
          {
            if ((a[k] < x))
            {
              return cpp_comma(puts("No"), 0);
            }
            x = a[k];
          }
          if ((a[k] > k))
          {
            if ((a[k] < y))
            {
              return cpp_comma(puts("No"), 0);
            }
            y = a[k];
          }
          k += 1;
        }
      }
      i = cpp_assign(j, "=", (j + 1));
    }
  }
  return cpp_comma(puts("Yes"), 0);
}
