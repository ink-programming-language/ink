// Translated from solution.cpp.

var year = cpp_array(30);

func poww(b: dynamic)
{
  var ans = 1;
  {
    var i = 0;
    while ((i < b))
    {
      ans *= 10;
      i += 1;
    }
  }
  return ans;
}

func compute(x: dynamic, pos: dynamic)
{
  var buf = cpp_array(30);
  strcpy(buf, (x + pos));
  var y = year[(pos + 1)];
  var dig = atoi(buf);
  if ((strlen(buf) == 1))
  {
    if ((dig == 9))
    {
      year[pos] = 1989;
    } else
    {
      year[pos] = (1990 + dig);
    }
  } else
  {
    var b = poww(strlen(buf));
    while ((dig <= y))
    {
      dig += b;
    }
    year[pos] = dig;
  }
}

func main()
{
  var n: dynamic;
  scanf("%I64d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      var next = cpp_array(30);
      scanf("%s", next);
      var len = strlen(next);
      {
        var i = 0;
        while ((i < (len - 4)))
        {
          compute(next, ((len - 1) - i));
          i += 1;
        }
      }
      printf("%I64d\n", year[4]);
      i += 1;
    }
  }
}
