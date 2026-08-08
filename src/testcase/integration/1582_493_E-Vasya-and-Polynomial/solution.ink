// Translated from solution.cpp.

var t: dynamic;

var a: dynamic;

var b: dynamic;

func main()
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
      var flag = 0;
      var p = 1;
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
  var cnt = 0;
  var p = 0;
  var q = 1;
  var r = b;
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
