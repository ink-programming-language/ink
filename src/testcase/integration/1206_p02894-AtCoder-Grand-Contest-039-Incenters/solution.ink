// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var Maxn: dynamic = cpp_expression("#inc");

var pi: dynamic = cpp_expression("#include");

var T: dynamic = cpp_array(Maxn);

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

func add(rad: dynamic, cnt: dynamic) -> dynamic
{
  x += (cos(rad) * cnt);
  y += (sin(rad) * cnt);
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&L));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lf", (&T[i]));
      T[i] = (((T[i] / L) * 2) * pi);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          var m1: dynamic = (((T[i] + T[j])) / 2);
          var m2: dynamic = (m1 + pi);
          add(m1, (((i + n) - j) - 1));
          add(m2, ((j - i) - 1));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var all: dynamic = ((((1.0 * n) * ((n - 1))) * ((n - 2))) / 6);
  x /= all;
  y /= all;
  printf("%.11lf\n %.11lf\n", x, y);
  return 0;
}
