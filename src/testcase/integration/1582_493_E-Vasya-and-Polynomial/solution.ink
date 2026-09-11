// Translated from solution.cpp.

var t: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(t, a, b);
  if ((t == 1))
  {
    if (((a == 1) && (b == 1)))
    {
      write("inf", "\n");
      return 0;
    } else if ((a == 1))
    {
      write(0, "\n");
      return 0;
    } else
    {
      var flag: dynamic = 0;
      var p: dynamic = 1;
      while ((p <= (b / a)))
      {
        p *= a;
        if ((p == b))
        {
          flag = 1;
        }
      }
      if (flag)
      {
        write(1, "\n");
        return 0;
      }
    }
  }
  var cnt: dynamic = 0;
  var p: dynamic = 0;
  var q: dynamic = 1;
  var r: dynamic = b;
  while ((r && (cnt < 100)))
  {
    p += ((r % a) * q);
    r /= a;
    q *= t;
    cnt += 1;
  }
  write((((p == a)) + (((cnt > 1) && (a == b)))), "\n");
  return 0;
}
