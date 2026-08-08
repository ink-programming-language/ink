// Translated from solution.cpp.

var a = cpp_array(100050);

var b = cpp_array(100);

func judge(la: dynamic, lb: dynamic)
{
  var s = 0;
  {
    var i = 0;
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

func main()
{
  while ((~scanf("%s%s", a, b)))
  {
    var la = strlen(a);
    var lb = strlen(b);
    var sum = 0;
    {
      var i = 0;
      while ((i < la))
      {
        if ((a[i] == b[0]))
        {
          var result = judge(i, lb);
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
