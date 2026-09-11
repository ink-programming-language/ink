// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_array(10);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  memset(num, 0, cpp_sizeof((num)));
  {
    i = 0;
    while ((i < 4))
    {
      getchar();
      {
        j = 0;
        while ((j < 4))
        {
          scanf("%c", (&c));
          if (isdigit(c))
          {
            num[(c - 48)] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    flag = true;
    while ((i < 10))
    {
      if (((n * 2) < num[i]))
      {
        printf("NO\n");
        flag = false;
        break;
      }
      i += 1;
    }
  }
  if (flag)
  {
    printf("YES\n");
  }
}
