// Translated from solution.cpp.

var EPS = 1e-9;

var PI = acos(-1.0);

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

func FOR(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i < (int)(n); i++)");
}

func FOREQ(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i <= (int)(n); i++)");
}

func FORIT(it: dynamic, c: dynamic)
{
  cpp_macro("for (__typeof((c).begin())it = (c).begin(); it != (c).end(); it++)");
}

func MEMSET(v: dynamic, h: dynamic)
{
  return cpp_expression("#include <stdio.h> #inclu");
}

var n: dynamic;

var point = cpp_array(200);

var radian = cpp_array(1000);

var used = cpp_array(1000);

func calc(index: dynamic)
{
  var m = 0;
  sort(radian, (radian + m));
  MEMSET(used, false);
  var lsum = 1;
  var ret = 1;
  REP(iter, 2);
  {
  }
  return ret;
}

func main()
{
  while (cpp_comma(scanf("%d", (&n)), n))
  {
    var ans = 1;
    printf("%d\n", ans);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var d = (abs((point[index] - point[i])) / 2.0);
    if (((i == index) || ((d - 1.0) > EPS)))
    {
      continue;
    }
    var center = (((point[index] + point[i])) / 2.0);
    var vect = (((point[i] - point[index])) * Point(0, -1));
    vect /= abs(vect);
    var l = sqrt((1 - (d * d)));
    if (isnan(l))
    {
      l = EPS;
    }
    var c1 = (center + (vect * l));
    var c2 = (center - (vect * l));
    var arg1 = (arg((c1 - point[index])) - EPS);
    var arg2 = (arg((c2 - point[index])) + EPS);
    radian[cpp_update(m, "++")] = make_pair(arg1, ((i + 1)));
    radian[cpp_update(m, "++")] = make_pair(arg2, (-((i + 1))));
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var p = (abs(radian[i].second) - 1);
      var start = if ((radian[i].second > 0)) true else false;
      if (start)
      {
        used[p] = true;
        lsum += 1;
      } else if (used[p])
      {
        used[p] = false;
        lsum -= 1;
      }
      ret = max(ret, lsum);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var x: dynamic;
      var y: dynamic;
      scanf("%lf,%lf", (&x), (&y));
      point[i] = Point(x, y);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      ans = max(ans, calc(i));
    }
