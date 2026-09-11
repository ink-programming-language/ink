// Translated from solution.cpp.

func calc(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic) -> dynamic
{
  if (((((((x2 - x1) + 1)) * (((y2 - y1) + 1))) % 2) == 0))
  {
    return 1;
  }
  return 2;
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    var x1: dynamic = cpp_uninitialized();
    var y1: dynamic = cpp_uninitialized();
    var x2: dynamic = cpp_uninitialized();
    var y2: dynamic = cpp_uninitialized();
    read(x1, y1, x2, y2);
    var x3: dynamic = cpp_uninitialized();
    var y3: dynamic = cpp_uninitialized();
    var x4: dynamic = cpp_uninitialized();
    var y4: dynamic = cpp_uninitialized();
    read(x3, y3, x4, y4);
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var sum_a: dynamic = cpp_uninitialized();
    var sum_b: dynamic = cpp_uninitialized();
    if ((calc(1, 1, m, n) == 1))
    {
      sum_a = cpp_assign(sum_b, "=", ((n * m) / 2));
    } else
    {
      sum_a = (((n * m) / 2) + 1);
      sum_b = ((n * m) / 2);
    }
    if ((calc(x1, y1, x2, y2) == 1))
    {
      a = (((((x2 - x1) + 1)) * (((y2 - y1) + 1))) / 2);
    } else
    {
      if (((x1 % 2) == (y1 % 2)))
      {
        a = (((((x2 - x1) + 1)) * (((y2 - y1) + 1))) / 2);
      } else
      {
        a = ((((((x2 - x1) + 1)) * (((y2 - y1) + 1))) / 2) + 1);
      }
    }
    if ((calc(x3, y3, x4, y4) == 1))
    {
      b = (((((x4 - x3) + 1)) * (((y4 - y3) + 1))) / 2);
    } else
    {
      if (((x3 % 2) == (y3 % 2)))
      {
        b = ((((((x4 - x3) + 1)) * (((y4 - y3) + 1))) / 2) + 1);
      } else
      {
        b = (((((x4 - x3) + 1)) * (((y4 - y3) + 1))) / 2);
      }
    }
    sum_a += a;
    sum_a -= b;
    sum_b -= a;
    sum_b += b;
    var tmpx: dynamic = max(x1, x3);
    var tmpy: dynamic = max(y1, y3);
    var tmpxx: dynamic = min(x2, x4);
    var tmpyy: dynamic = min(y2, y4);
    if (((tmpx > tmpxx) || (tmpy > tmpyy)))
    {
    } else
    {
      var c: dynamic = cpp_uninitialized();
      if ((calc(tmpx, tmpy, tmpxx, tmpyy) == 1))
      {
        c = (((((tmpxx - tmpx) + 1)) * (((tmpyy - tmpy) + 1))) / 2);
      } else
      {
        if (((tmpx % 2) == (tmpy % 2)))
        {
          c = (((((tmpxx - tmpx) + 1)) * (((tmpyy - tmpy) + 1))) / 2);
        } else
        {
          c = ((((((tmpxx - tmpx) + 1)) * (((tmpyy - tmpy) + 1))) / 2) + 1);
        }
      }
      sum_b += c;
      sum_a -= c;
    }
    write(sum_a, " ", sum_b, "\n");
  }
}
