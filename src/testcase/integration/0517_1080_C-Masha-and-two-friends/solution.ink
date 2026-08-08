// Translated from solution.cpp.

func calc(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
{
  if (((((((x2 - x1) + 1)) * (((y2 - y1) + 1))) % 2) == 0))
  {
    return 1;
  }
  return 2;
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    var x1: dynamic;
    var y1: dynamic;
    var x2: dynamic;
    var y2: dynamic;
    read(x1, y1, x2, y2);
    var x3: dynamic;
    var y3: dynamic;
    var x4: dynamic;
    var y4: dynamic;
    read(x3, y3, x4, y4);
    var a: dynamic;
    var b: dynamic;
    var sum_a: dynamic;
    var sum_b: dynamic;
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
    var tmpx = max(x1, x3);
    var tmpy = max(y1, y3);
    var tmpxx = min(x2, x4);
    var tmpyy = min(y2, y4);
    if (((tmpx > tmpxx) || (tmpy > tmpyy)))
    {
    } else
    {
      var c: dynamic;
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
