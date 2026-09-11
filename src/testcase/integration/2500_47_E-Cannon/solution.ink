// Translated from solution.cpp.

func show(a: dynamic, n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func show(a: dynamic, r: dynamic, l: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < r))
    {
      show(a[i], l);
      i += 1;
    }
  }
  write("\n");
}

var N: dynamic = 120000;

var M: dynamic = 120000;

var oo: dynamic = ((10000 * 10000) * 10);

var g: dynamic = 9.8;

var ang: dynamic = cpp_array(N);

var wx: dynamic = cpp_array(M);

var wy: dynamic = cpp_array(M);

var ax: dynamic = cpp_array(N);

var ay: dynamic = cpp_array(N);

var pd: dynamic = cpp_array(N);

var wd: dynamic = cpp_array(M);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

func pcmp(i: dynamic, j: dynamic) -> dynamic
{
  return (ang[i] < ang[j]);
}

func wcmp(i: dynamic, j: dynamic) -> dynamic
{
  return (wx[i] < wx[j]);
}

func solve() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
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
      var id: dynamic = pd[i];
      var vx: dynamic = (v * cos(ang[id]));
      var vy: dynamic = (v * sin(ang[id]));
      var t: dynamic = ((2.0 * vy) / g);
      var sx: dynamic = (vx * t);
      while ((j < m))
      {
        var jd: dynamic = wd[j];
        if ((wx[jd] <= sx))
        {
          var tt: dynamic = (wx[jd] / vx);
          var ty: dynamic = ((vy * tt) - (((0.5 * g) * tt) * tt));
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

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var cas: dynamic = 0;
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
