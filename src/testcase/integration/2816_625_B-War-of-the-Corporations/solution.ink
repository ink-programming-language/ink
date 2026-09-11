// Translated from solution.cpp.

var a: dynamic = cpp_array(100050);

var b: dynamic = cpp_array(100);

func judge(la: dynamic, lb: dynamic) -> dynamic
{
  var s: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < lb))
    {
      if ((a[la] == b[i]))
      {
        s += 1;
        la += 1;
      } else
      {
        break;
      }
      i += 1;
    }
  }
  if ((s == lb))
  {
    return 1;
  } else
  {
    return 0;
  }
}

func main() -> dynamic
{
  while ((~scanf("%s%s", a, b)))
  {
    var la: dynamic = strlen(a);
    var lb: dynamic = strlen(b);
    var sum: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < la))
      {
        if ((a[i] == b[0]))
        {
          var result: dynamic = judge(i, lb);
          if ((result == 1))
          {
            sum += 1;
            i = ((i + lb) - 1);
          }
        }
        i += 1;
      }
    }
    printf("%d\n", sum);
  }
  return 0;
}
