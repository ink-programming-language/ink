// Translated from solution.cpp.

var N: dynamic = 2005;

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(8000005);

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      cnt[a[i]] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (b + i));
      cnt[b[i]] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((cnt[(a[i] ^ b[j])] == 1))
          {
            ans += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((ans & 1))
  {
    printf("Koyomi\n");
  } else
  {
    printf("Karen\n");
  }
}
