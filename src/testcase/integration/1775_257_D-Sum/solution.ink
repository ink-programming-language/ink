// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%I64d", (&v[i]));
      i += 1;
    }
  }
  ans[(n - 1)] = cpp_char("+");
  var now: dynamic = v[(n - 1)];
  {
    var i: dynamic = (n - 2);
    while ((i >= 0))
    {
      if ((v[i] >= now))
      {
        ans[i] = cpp_char("+");
        now = (v[i] - now);
      } else
      {
        ans[i] = cpp_char("-");
        now -= v[i];
      }
      i -= 1;
    }
  }
  var change: dynamic = false;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (change)
      {
        if ((ans[i] == cpp_char("+")))
        {
          printf("-");
        } else
        {
          printf("+");
        }
      } else
      {
        printf("%c", ans[i]);
      }
      if ((ans[i] == cpp_char("+")))
      {
        change = (!change);
      }
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
