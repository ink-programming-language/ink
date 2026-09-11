// Translated from solution.cpp.

var USE_MATH_DEFINES: dynamic = cpp_expression("#def");

var CRT_SECURE_NO_DEPRECATE: dynamic = cpp_expression("#def");

func FOR(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < (n); i++)");
}

func sz(c: dynamic) -> dynamic
{
  return cpp_expression("#define _USE_MATH");
}

func ten(x: dynamic) -> dynamic
{
  return cpp_expression("#define _USE");
}

func tenll(x: dynamic) -> dynamic
{
  return cpp_expression("#define _US");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var v: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_array(2, 2);
  ans[0][0] = cpp_assign(ans[0][1], "=", cpp_assign(ans[1][0], "=", cpp_assign(ans[1][1], "=", -1)));
  FOR(i, (n - 1));
  {
    var pf: dynamic = cpp_uninitialized();
    var pa: dynamic = cpp_uninitialized();
    var pt: dynamic = cpp_uninitialized();
    var px: dynamic = cpp_uninitialized();
    var py: dynamic = cpp_uninitialized();
    var f: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    tie(pf, pa, pt, px, py) = v[i];
    tie(f, a, t, x, y) = v[(i + 1)];
    if (((pt == t) && (a != pa)))
    {
      var nt: dynamic = sqrt((pow((x - px), 2) + pow((y - py), 2)));
      if ((ans[t][0] <= nt))
      {
        if ((abs((ans[t][0] - nt)) < 1e-6))
        {
          ans[t][1] = min(ans[t][1], (((f - pf)) / 60.0));
        } else
        {
          ans[t][1] = (((f - pf)) / 60.0);
        }
        ans[t][0] = nt;
      }
    }
  }
  FOR(i, 2);
  printf("%.10lf %.10lf\n", ans[i][0], ans[i][1]);
}

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var f: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    read(f, a, t, x, y);
    v.emplace_back(f, a, t, x, y);
  }
