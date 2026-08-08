// Translated from solution.cpp.

var x = cpp_expression("#incl");

var y = cpp_expression("#inclu");

var mp = cpp_expression("#include");

var pb = cpp_expression("#include");

func enum_cpp(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i=(x);i<=(y);++i)");
}

func try_cpp(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i=(x);i>=(y);--i)");
}

func chkmax(x: dynamic, y: dynamic)
{
  if ((x < y)) cpp_assign(x, "=", y) else 0;
}

func chkmin(x: dynamic, y: dynamic)
{
  if ((y < x)) cpp_assign(x, "=", y) else 0;
}

func readint(x: dynamic)
{
  x = 0;
  var f = 1;
  var c: dynamic;
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

var MAXN = 105;

var pi = (atan(1) * 4);

var n: dynamic;

var x = cpp_array(MAXN);

var y = cpp_array(MAXN);

var ang = cpp_array(MAXN);

func solve(cur: dynamic)
{
  cpp_statement("enum(i,1,n)");
  {
    if ((i == cur))
    {
      ang[i] = 0;
      continue;
    }
    var x0 = (x[i] - x[cur]);
    var y0 = (y[i] - y[cur]);
    if ((fabs(x0) < 1e-8))
    {
      ang[i] = if ((y0 > 0)) (pi / 2) else ((-pi) / 2);
    } else
    {
      ang[i] = (atan((y0 / x0)) + (if ((x0 < 0)) pi else 0));
    }
    if ((ang[i] < 0))
    {
      ang[i] += (2 * pi);
    }
  }
  sort((ang + 1), ((ang + n) + 1));
  ang[1] = (ang[n] - (2 * pi));
  var ans = 0;
  cpp_statement("enum");
  (cpp_comma(i, cpp_comma(2, n)));
  chkmax(ans, (ang[i] - ang[(i - 1)]));
  return if ((ans < pi)) 0 else ((((ans / pi) - 1)) * 0.5);
}

func main()
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
