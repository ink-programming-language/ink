// Translated from solution.cpp.

var N = 2005;

var n: dynamic;

var ans: dynamic;

var a = cpp_array(N);

var b = cpp_array(N);

var cnt = cpp_array(8000005);

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      cnt[a[i]] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (b + i));
      cnt[b[i]] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
