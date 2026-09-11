// Translated from solution.cpp.

var MAXN: dynamic = 1605;

var eps: dynamic = 1e-10;

class data
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  func operator_less(d2: dynamic) -> dynamic
  {
      return (((t * p) * ((1 - d2.p))) < ((d2.t * d2.p) * ((1 - p))));
    }
}

var a: dynamic = cpp_array(MAXN);

var N: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(MAXN, MAXN);

var g: dynamic = cpp_array(MAXN, MAXN);

var ans: dynamic = -1e100;

var tim: dynamic = 1e100;

func main() -> dynamic
{
  read(N, T);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(a[i].a, a[i].b, a[i].s, a[i].t, a[i].p);
      i += 1;
    }
  }
  sort((a + 1), ((a + N) + 1));
  {
    var i: dynamic = 1;
    while ((i <= T))
    {
      f[0][i] = -1e100;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      {
        var j: dynamic = 0;
        while ((j <= T))
        {
          f[i][j] = f[(i - 1)][j];
          g[i][j] = g[(i - 1)][j];
          j += 1;
        }
      }
      {
        var j: dynamic = a[i].s;
        while ((j <= T))
        {
          if ((((f[(i - 1)][(j - a[i].s)] + a[i].a) - f[i][j]) > eps))
          {
            f[i][j] = (f[(i - 1)][(j - a[i].s)] + a[i].a);
            g[i][j] = g[(i - 1)][(j - a[i].s)];
          } else if ((abs(((f[(i - 1)][(j - a[i].s)] + a[i].a) - f[i][j])) < eps))
          {
            g[i][j] = max(g[i][j], g[(i - 1)][(j - a[i].s)]);
          }
          j += 1;
        }
      }
      {
        var j: dynamic = (a[i].s + a[i].t);
        while ((j <= T))
        {
          if (((((f[(i - 1)][((j - a[i].s) - a[i].t)] + a[i].a) + (a[i].b * ((1 - a[i].p)))) - f[i][j]) > eps))
          {
            f[i][j] = ((f[(i - 1)][((j - a[i].s) - a[i].t)] + a[i].a) + (a[i].b * ((1 - a[i].p))));
            g[i][j] = (((g[(i - 1)][((j - a[i].s) - a[i].t)] + a[i].t)) * a[i].p);
          } else if ((abs((((f[(i - 1)][((j - a[i].s) - a[i].t)] + a[i].a) + (a[i].b * ((1 - a[i].p)))) - f[i][j])) < eps))
          {
            g[i][j] = max(g[i][j], (((g[(i - 1)][((j - a[i].s) - a[i].t)] + a[i].t)) * a[i].p));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= T))
    {
      if (((f[N][i] - ans) > eps))
      {
        ans = f[N][i];
        tim = (i - g[N][i]);
      } else if ((abs((f[N][i] - ans)) < eps))
      {
        tim = min(tim, (i - g[N][i]));
      }
      i += 1;
    }
  }
  write(fixed, setprecision(10), ans, cpp_char(" "), tim);
  return 0;
}
