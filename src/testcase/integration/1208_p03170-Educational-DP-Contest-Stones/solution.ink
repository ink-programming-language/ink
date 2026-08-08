// Translated from solution.cpp.

var dp = cpp_array(100010);

var a = cpp_array(1001);

func main()
{
  var n: dynamic;
  var k: dynamic;
  scanf("%d %d", (&n), (&k));
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
    while ((i <= k))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if (((i >= a[j]) && (!dp[(i - a[j])])))
          {
            dp[i] = 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if (dp[k])
  {
    puts("First");
  } else
  {
    puts("Second");
  }
  return 0;
}
