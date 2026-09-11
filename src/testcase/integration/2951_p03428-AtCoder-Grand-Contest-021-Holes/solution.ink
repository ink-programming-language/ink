// Translated from solution.cpp.

var x: dynamic = cpp_expression("#incl");

var y: dynamic = cpp_expression("#inclu");

var mp: dynamic = cpp_expression("#include");

var pb: dynamic = cpp_expression("#include");

func enum_cpp(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i=(x);i<=(y);++i)");
}

func try_cpp(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i=(x);i>=(y);--i)");
}

func chkmax(x: dynamic, y: dynamic) -> dynamic
{
   ((x < y)) ? cpp_assign(x, "=", y) : 0;
}

func chkmin(x: dynamic, y: dynamic) -> dynamic
{
   ((y < x)) ? cpp_assign(x, "=", y) : 0;
}

func readint(x: dynamic) -> dynamic
{
  x = 0;
  var f: dynamic = 1;
  var c: dynamic = cpp_uninitialized();
  {
    c = getchar();
    while ((!isdigit(c)))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      }
      c = getchar();
    }
  }
  {
    while (isdigit(c))
    {
      x = (((x * 10) + c) - cpp_char("0"));
      c = getchar();
    }
  }
  x *= f;
}

var MAXN: dynamic = 105;

var pi: dynamic = (atan(1) * 4);

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_array(MAXN);

var y: dynamic = cpp_array(MAXN);

var ang: dynamic = cpp_array(MAXN);

func solve(cur: dynamic) -> dynamic
{
  cpp_statement("enum(i,1,n)");
  {
    if ((i == cur))
    {
      ang[i] = 0;
      continue;
    }
    var x0: dynamic = (x[i] - x[cur]);
    var y0: dynamic = (y[i] - y[cur]);
    if ((fabs(x0) < 1e-8))
    {
      ang[i] =  ((y0 > 0)) ? (pi / 2) : ((-pi) / 2);
    } else
    {
      ang[i] = (atan((y0 / x0)) + ( ((x0 < 0)) ? pi : 0));
    }
    if ((ang[i] < 0))
    {
      ang[i] += (2 * pi);
    }
  }
  sort((ang + 1), ((ang + n) + 1));
  ang[1] = (ang[n] - (2 * pi));
  var ans: dynamic = 0;
  cpp_statement("enum");
  (cpp_comma(i, cpp_comma(2, n)));
  chkmax(ans, (ang[i] - ang[(i - 1)]));
  return  ((ans < pi)) ? 0 : ((((ans / pi) - 1)) * 0.5);
}

func main() -> dynamic
{
  readint(n);
  cpp_statement("enum");
  (cpp_comma(i, cpp_comma(1, n)));
  scanf("%lf%lf", (&x[i]), (&y[i]));
  cpp_statement("enum");
  (cpp_comma(i, cpp_comma(1, n)));
  printf("%.10lf\n", solve(i));
  return 0;
}
