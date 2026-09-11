// Translated from solution.cpp.

var dp: dynamic = cpp_array(100010);

var a: dynamic = cpp_array(1001);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  scanf("%d %d", (&n), (&k));
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
    while ((i <= k))
    {
      {
        var j: dynamic = 0;
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
