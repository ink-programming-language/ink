// Translated from solution.cpp.

var n: dynamic;

var L: dynamic;

var Maxn = cpp_expression("#inc");

var pi = cpp_expression("#include");

var T = cpp_array(Maxn);

var x: dynamic;

var y: dynamic;

func add(rad: dynamic, cnt: dynamic)
{
  x += (cos(rad) * cnt);
  y += (sin(rad) * cnt);
}

func main()
{
  scanf("%d%d", (&n), (&L));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%lf", (&T[i]));
      T[i] = (((T[i] / L) * 2) * pi);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
        while ((j < n))
        {
          var m1 = (((T[i] + T[j])) / 2);
          var m2 = (m1 + pi);
          add(m1, (((i + n) - j) - 1));
          add(m2, ((j - i) - 1));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var all = ((((1.0 * n) * ((n - 1))) * ((n - 2))) / 6);
  x /= all;
  y /= all;
  printf("%.11lf\n %.11lf\n", x, y);
  return 0;
}
