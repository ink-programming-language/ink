// Translated from solution.cpp.

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  while (((cin >> n) >> k))
  {
    var d: dynamic = (n + 1);
    var ara: dynamic = cpp_array(d);
    var t1: dynamic = 0;
    var c: dynamic = 0;
    var t: dynamic = cpp_uninitialized();
    var temp: dynamic = 0;
    var is: dynamic = 1;
    {
      i = 1;
      while ((i <= n))
      {
        read(ara[i]);
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= n))
      {
        ara[i] = (temp + ara[i]);
        if ((ara[i] >= k))
        {
          temp = (ara[i] % k);
          c += (ara[i] / k);
        } else if (((ara[i] > 0) && (temp > 0)))
        {
          c += 1;
          temp = 0;
        } else
        {
          temp = ara[i];
        }
        i += 1;
      }
    }
    if ((temp > 0))
    {
      write((c + 1), "\n");
    } else
    {
      write(c, "\n");
    }
  }
  return 0;
}
