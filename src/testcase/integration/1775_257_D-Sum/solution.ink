// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%I64d", (&v[i]));
      i += 1;
    }
  }
  ans[(n - 1)] = cpp_char("+");
  var now = v[(n - 1)];
  {
    var i = (n - 2);
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
  var change = false;
  {
    var i = 0;
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
