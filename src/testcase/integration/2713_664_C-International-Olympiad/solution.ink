// Translated from solution.cpp.

var year: dynamic = cpp_array(30);

func poww(b: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < b))
    {
      ans *= 10;
      i += 1;
    }
  }
  return ans;
}

func compute(x: dynamic, pos: dynamic) -> dynamic
{
  var buf: dynamic = cpp_array(30);
  strcpy(buf, (x + pos));
  var y: dynamic = year[(pos + 1)];
  var dig: dynamic = atoi(buf);
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
    var b: dynamic = poww(strlen(buf));
    while ((dig <= y))
    {
      dig += b;
    }
    year[pos] = dig;
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%I64d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var next: dynamic = cpp_array(30);
      scanf("%s", next);
      var len: dynamic = strlen(next);
      {
        var i: dynamic = 0;
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
