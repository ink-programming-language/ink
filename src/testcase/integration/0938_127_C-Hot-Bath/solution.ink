// Translated from solution.cpp.

var t1: dynamic;

var t2: dynamic;

var x1: dynamic;

var x2: dynamic;

var t0: dynamic;

func main()
{
  var i: dynamic;
  while ((((((cin >> t1) >> t2) >> x1) >> x2) >> t0))
  {
    if (((t2 == t0) && (t0 != t1)))
    {
      write("0", " ", x2, "\n");
      continue;
    }
    if (((t2 != t0) && (t0 == t1)))
    {
      write(x1, " ", "0", "\n");
      continue;
    }
    if (((t2 == t0) && (t0 == t1)))
    {
      write(x1, " ", x2, "\n");
      continue;
    }
    var y1: dynamic;
    var y2: dynamic;
    var tt = 99999999;
    var tmp: dynamic;
    {
      i = 0;
      while ((i <= x1))
      {
        if ((i == 0))
        {
          tmp = x2;
        } else
        {
          tmp = (i * ((t0 - t1)));
          if (((tmp % ((t2 - t0))) != 0))
          {
            tmp = (((tmp / ((t2 - t0)))) + 1);
          } else
          {
            tmp = ((tmp / ((t2 - t0))));
          }
          if (((tmp > x2) || (tmp < 0)))
          {
            i += 1;
            continue;
          }
          if ((tmp < 0))
          {
            while (1)
            {
            }
          }
        }
        var t: dynamic;
        t = (((((((tmp * 1.0) * t2) * 1.0) + (((i * 1.0) * t1) * 1.0))) * 1.0) / (((i * 1.0) + (tmp * 1.0))));
        if (((t >= (t0 * 1.0)) && (t < tt)))
        {
          y1 = i;
          y2 = tmp;
          tt = t;
        } else if (((t >= (t0 * 1.0)) && (fabs((t - tt)) < 1e-20)))
        {
          if (((i + tmp) > (y1 + y2)))
          {
            y1 = i;
            y2 = tmp;
            tt = t;
          }
        }
        i += 1;
      }
    }
    write(y1, " ", y2, "\n");
  }
  return 0;
}
