// Translated from solution.cpp.

func show(a: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func show(a: dynamic, r: dynamic, l: dynamic)
{
  {
    var i = 0;
    while ((i < r))
    {
      show(a[i], l);
      i += 1;
    }
  }
  write("\n");
}

var N = 120000;

var M = 120000;

var oo = ((10000 * 10000) * 10);

var g = 9.8;

var ang = cpp_array(N);

var wx = cpp_array(M);

var wy = cpp_array(M);

var ax = cpp_array(N);

var ay = cpp_array(N);

var pd = cpp_array(N);

var wd = cpp_array(M);

var n: dynamic;

var m: dynamic;

var v: dynamic;

func pcmp(i: dynamic, j: dynamic)
{
  return (ang[i] < ang[j]);
}

func wcmp(i: dynamic, j: dynamic)
{
  return (wx[i] < wx[j]);
}

func solve()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  {
    i = 0;
    while ((i < n))
    {
      pd[i] = i;
      i += 1;
    }
  }
  {
    j = 0;
    while ((j < m))
    {
      wd[j] = j;
      j += 1;
    }
  }
  sort(pd, (pd + n), pcmp);
  sort(wd, (wd + m), wcmp);
  j = 0;
  {
    i = 0;
    while ((i < n))
    {
      var id = pd[i];
      var vx = (v * cos(ang[id]));
      var vy = (v * sin(ang[id]));
      var t = ((2.0 * vy) / g);
      var sx = (vx * t);
      while ((j < m))
      {
        var jd = wd[j];
        if ((wx[jd] <= sx))
        {
          var tt = (wx[jd] / vx);
          var ty = ((vy * tt) - (((0.5 * g) * tt) * tt));
          if ((ty <= wy[jd]))
          {
            ax[id] = wx[jd];
            ay[id] = ty;
            break;
          } else
          {
            j += 1;
          }
        } else
        {
          ax[id] = sx;
          ay[id] = 0;
          break;
        }
      }
      if ((j == m))
      {
        ax[id] = sx;
        ay[id] = 0;
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      printf("%.10f %.10f\n", ax[i], ay[i]);
      i += 1;
    }
  }
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var cas = 0;
  scanf("%d", (&n));
  scanf("%lf", (&v));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%lf", (&ang[i]));
      i += 1;
    }
  }
  scanf("%d", (&m));
  {
    i = 0;
    while ((i < m))
    {
      scanf("%lf %lf", (&wx[i]), (&wy[i]));
      i += 1;
    }
  }
  solve();
  return 0;
}
