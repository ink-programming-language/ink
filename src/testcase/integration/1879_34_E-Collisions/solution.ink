// Translated from solution.cpp.

var n: dynamic;

var m = cpp_array(15);

var x = cpp_array(15);

var v = cpp_array(15);

var t: dynamic;

var v1: dynamic;

var v2: dynamic;

var now: dynamic;

var pre: dynamic;

var tmp: dynamic;

var eps = 1e-10;

func main()
{
  scanf("%d%lf", (&n), (&t));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lf%lf%d", (&x[i]), (&v[i]), (&m[i]));
      i += 1;
    }
  }
  {
    while (true)
    {
      now = (2 * t);
      {
        var i = 1;
        while ((i <= n))
        {
          {
            var j = (i + 1);
            while ((j <= n))
            {
              if (((fabs((x[i] - x[j])) < eps) || (fabs((v[i] - v[j])) < eps)))
              {
                j += 1;
                continue;
              }
              tmp = (((x[j] - x[i])) / ((v[i] - v[j])));
              if ((tmp < eps))
              {
                j += 1;
                continue;
              }
              now = min(now, tmp);
              j += 1;
            }
          }
          i += 1;
        }
      }
      if (((now + pre) > t))
      {
        break;
      }
      pre += now;
      {
        var i = 1;
        while ((i <= n))
        {
          x[i] += (now * v[i]);
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= n))
        {
          {
            var j = (i + 1);
            while ((j <= n))
            {
              if ((fabs((x[i] - x[j])) < eps))
              {
                v1 = v[i];
                v2 = v[j];
                v[i] = ((((((m[i] - m[j])) * v1) + ((2 * m[j]) * v2))) / ((m[i] + m[j])));
                v[j] = ((((((m[j] - m[i])) * v2) + ((2 * m[i]) * v1))) / ((m[i] + m[j])));
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  }
  now = (t - pre);
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%.12lf\n", (x[i] + (v[i] * now)));
      i += 1;
    }
  }
}
