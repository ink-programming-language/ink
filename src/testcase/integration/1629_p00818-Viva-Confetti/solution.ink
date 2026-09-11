// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func each(itr: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof(c.begin()) itr=c.begin(); itr!=c.end(); ++itr)");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

var pb: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var EPS: dynamic = 1e-18;

var x: dynamic = cpp_array(100);

var y: dynamic = cpp_array(100);

var r: dynamic = cpp_array(100);

var cross: dynamic = cpp_uninitialized();

func dist(i: dynamic, j: dynamic) -> dynamic
{
  return sqrt(((((x[i] - x[j])) * ((x[i] - x[j]))) + (((y[i] - y[j])) * ((y[i] - y[j])))));
}

func isCovered(i: dynamic, j: dynamic) -> dynamic
{
  if ((r[j] > r[i]))
  {
    return (dist(i, j) <= (r[j] - r[i]));
  }
  return false;
}

func calcCrossPoints(i: dynamic, j: dynamic) -> dynamic
{
  if ((((dist(i, j) > ((r[i] + r[j]))) || isCovered(i, j)) || isCovered(j, i)))
  {
    return vector();
  }
  var ret: dynamic = cpp_uninitialized();
  var A: dynamic = (2 * ((x[j] - x[i])));
  var B: dynamic = (2 * ((y[j] - y[i])));
  var C: dynamic = ((((((x[i] * x[i]) - (x[j] * x[j])) + (y[i] * y[i])) - (y[j] * y[j])) - (r[i] * r[i])) + (r[j] * r[j]));
  if ((fabs(B) < EPS))
  {
    var X: dynamic = ((-C) / A);
    var D: dynamic = ((r[i] * r[i]) - (((X - x[i])) * ((X - x[i]))));
    ret.pb(pd(X, (y[i] - sqrt(D))));
    ret.pb(pd(X, (y[i] + sqrt(D))));
  } else
  {
    var a: dynamic = ((-A) / B);
    var b: dynamic = ((-C) / B);
    var P: dynamic = ((a * a) + 1);
    var Q: dynamic = (((a * b) - x[i]) - (a * y[i]));
    var R: dynamic = (((x[i] * x[i]) + (((b - y[i])) * ((b - y[i])))) - (r[i] * r[i]));
    var D: dynamic = ((Q * Q) - (P * R));
    var x1: dynamic = ((((-Q) - sqrt(D))) / P);
    var x2: dynamic = ((((-Q) + sqrt(D))) / P);
    var y1: dynamic = ((a * x1) + b);
    var y2: dynamic = ((a * x2) + b);
    ret.pb(pd(x1, y1));
    ret.pb(pd(x2, y2));
  }
  return ret;
}

func inCircle(i: dynamic, p: dynamic) -> dynamic
{
  var dx: dynamic = (x[i] - p.fi);
  var dy: dynamic = (y[i] - p.se);
  var d: dynamic = sqrt(((dx * dx) + (dy * dy)));
  return (d <= r[i]);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma(scanf(" %d", (&n)), n))
  {
    cpp_statement("rep(i,n) scanf(\" %lf %lf %lf\", &x[i], &y[i], &r[i]); cross.clear(); rep(i,n)");
    var ans: dynamic = 1;
    {
      var i: dynamic = (n - 2);
      while ((i >= 0))
      {
        var cov: dynamic = false;
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            if (isCovered(i, j))
            {
              cov = true;
            }
            j += 1;
          }
        }
        if (cov)
        {
          i -= 1;
          continue;
        }
        var exist: dynamic = false;
        rep(j, cross.size());
        {
          var a: dynamic = cross[j].se.fi;
          var b: dynamic = cross[j].se.se;
          if (((a < i) || (b < i)))
          {
            i -= 1;
            continue;
          }
          if (inCircle(i, cross[j].fi))
          {
            exist = true;
          }
        }
        if ((!exist))
        {
          ans += 1;
          i -= 1;
          continue;
        }
        rep(j, cross.size());
        {
          if ((!inCircle(i, cross[j].fi)))
          {
            i -= 1;
            continue;
          }
          var a: dynamic = cross[j].se.fi;
          var b: dynamic = cross[j].se.se;
          var hidden: dynamic = false;
          {
            var k: dynamic = (i + 1);
            while ((k < n))
            {
              if (((k == a) || (k == b)))
              {
                k += 1;
                continue;
              }
              if (inCircle(k, cross[j].fi))
              {
                hidden = true;
                break;
              }
              k += 1;
            }
          }
          if ((!hidden))
          {
            ans += 1;
            break;
          }
        }
        i -= 1;
      }
    }
    printf("%d\n", ans);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var cr: dynamic = calcCrossPoints(j, i);
      rep(k, cr.size()).pb(pp(cr[k], pi(j, i)));
    }
