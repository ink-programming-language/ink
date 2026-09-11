// Translated from solution.cpp.

var size: dynamic = cpp_array(112345);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var cur: dynamic = 0;
  scanf("%d", (&n));
  getchar();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var c: dynamic = getchar();
      if ((c == cpp_char(">")))
      {
        size[i] = 1;
      } else
      {
        size[i] = -1;
      }
      i += 1;
    }
  }
  getchar();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      scanf("%d", (&a));
      size[i] *= a;
      i += 1;
    }
  }
  {
    while (true)
    {
      if (((cur < 0) || (cur >= n)))
      {
        break;
      }
      if ((size[cur] == 0))
      {
        break;
      }
      var t: dynamic = cur;
      cur += size[cur];
      size[t] = 0;
    }
  }
  if (((cur < 0) || (cur >= n)))
  {
    printf("FINITE\n");
  } else
  {
    printf("INFINITE\n");
  }
  return 0;
}
