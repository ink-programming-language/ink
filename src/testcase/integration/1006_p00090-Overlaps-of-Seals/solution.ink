// Translated from solution.cpp.

var EPS: dynamic = 1e-9;

var PI: dynamic = acos(-1.0);

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

func FOR(i: dynamic, s: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (s); i < (int)(n); i++)");
}

func FOREQ(i: dynamic, s: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (s); i <= (int)(n); i++)");
}

func FORIT(it: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for (__typeof((c).begin())it = (c).begin(); it != (c).end(); it++)");
}

func MEMSET(v: dynamic, h: dynamic) -> dynamic
{
  return cpp_expression("#include <stdio.h> #inclu");
}

var n: dynamic = cpp_uninitialized();

var point: dynamic = cpp_array(200);

var radian: dynamic = cpp_array(1000);

var used: dynamic = cpp_array(1000);

func calc(index: dynamic) -> dynamic
{
  var m: dynamic = 0;
  sort(radian, (radian + m));
  MEMSET(used, false);
  var lsum: dynamic = 1;
  var ret: dynamic = 1;
  REP(iter, 2);
  {
  }
  return ret;
}

func main() -> dynamic
{
  while (cpp_comma(scanf("%d", (&n)), n))
  {
    var ans: dynamic = 1;
    printf("%d\n", ans);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var d: dynamic = (abs((point[index] - point[i])) / 2.0);
    if (((i == index) || ((d - 1.0) > EPS)))
    {
      continue;
    }
    var center: dynamic = (((point[index] + point[i])) / 2.0);
    var vect: dynamic = (((point[i] - point[index])) * Point(0, -1));
    vect /= abs(vect);
    var l: dynamic = sqrt((1 - (d * d)));
    if (isnan(l))
    {
      l = EPS;
    }
    var c1: dynamic = (center + (vect * l));
    var c2: dynamic = (center - (vect * l));
    var arg1: dynamic = (arg((c1 - point[index])) - EPS);
    var arg2: dynamic = (arg((c2 - point[index])) + EPS);
    radian[cpp_update(m, "++")] = make_pair(arg1, ((i + 1)));
    radian[cpp_update(m, "++")] = make_pair(arg2, (-((i + 1))));
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var p: dynamic = (abs(radian[i].second) - 1);
      var start: dynamic =  ((radian[i].second > 0)) ? true : false;
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

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%lf,%lf", (&x), (&y));
      point[i] = Point(x, y);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      ans = max(ans, calc(i));
    }
