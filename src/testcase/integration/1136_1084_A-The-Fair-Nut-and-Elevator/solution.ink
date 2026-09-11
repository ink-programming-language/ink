// Translated from solution.cpp.

var a: dynamic = cpp_array(200);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var pos: dynamic = 0;
  var maxx: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((maxx < a[i]))
      {
        maxx = a[i];
        pos = i;
      }
      i += 1;
    }
  }
  if ((maxx == 0))
  {
    return cpp_comma(puts("0"), 0);
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans += ((4 * a[i]) * ((i - 1)));
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
