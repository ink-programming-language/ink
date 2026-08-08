// Translated from solution.cpp.

func debug(x: dynamic)
{
  cpp_macro("cerr << #x << \" = \" << x << endl;");
}

var mod = cpp_expression("#include <");

var INF = cpp_expression("#include <");

var LLINF = cpp_expression("#include <cstdio> #include <");

var SIZE = cpp_expression("#inclu");

class P
{
  var x: dynamic;
  var y: dynamic;
  func P(X: dynamic, Y: dynamic)
  {
      x = X;
      y = Y;
    }
  func operator_less(B: dynamic)
  {
      return if ((x == B.x)) (y < B.y) else (x < B.x);
    }
}

func ccw(base: dynamic, A: dynamic, B: dynamic)
{
  return (((((A.x - base.x)) * ((B.y - base.y))) - (((A.y - base.y)) * ((B.x - base.x)))) >= 0);
}

func ConvexHull(s: dynamic)
{
  var g: dynamic;
  var n = cpp_cast(s.size());
  if ((n < 3))
  {
    return s;
  }
  sort(s.begin(), s.end());
  {
    var i = 0;
    while ((i < n))
    {
      {
        var m = cpp_cast(g.size());
        while (((m >= 2) && ccw(g[(m - 2)], g[(m - 1)], s[i])))
        {
          g.pop_back();
          m -= 1;
        }
      }
      g.push_back(s[i]);
      i += 1;
    }
  }
  var t = cpp_cast(g.size());
  {
    var i = (n - 2);
    while ((i >= 0))
    {
      {
        var m = cpp_cast(g.size());
        while (((m > t) && ccw(g[(m - 2)], g[(m - 1)], s[i])))
        {
          g.pop_back();
          m -= 1;
        }
      }
      g.push_back(s[i]);
      i -= 1;
    }
  }
  reverse(g.begin(), g.end());
  g.pop_back();
  return g;
}

func main()
{
  var n: dynamic;
  var t = cpp_array(SIZE);
  var d = cpp_array(SIZE);
  var y = cpp_array(SIZE);
  var x = cpp_array(SIZE);
  var s = [];
  var vec: dynamic;
  scanf("%d", (&n));
  y[0] = cpp_assign(x[0], "=", 0);
  var dx = [-1, 0, 1, 0];
  var dy = [0, 1, 0, -1];
  {
    var i = 1;
    while ((i < n))
    {
      scanf("%d%d", (t + i), (d + i));
      x[i] = (x[t[i]] + dx[d[i]]);
      y[i] = (y[t[i]] + dy[d[i]]);
      s[t[i]][d[i]] = true;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      vec.push_back(P(y[i], x[i]));
      vec.push_back(P(y[i], (x[i] + 1)));
      vec.push_back(P((y[i] - 1), (x[i] + 1)));
      vec.push_back(P((y[i] - 1), x[i]));
      i += 1;
    }
  }
  vec = ConvexHull(vec);
  var ans = LLINF;
  var theta: dynamic;
  {
    var i = 0;
    while ((i < vec.size()))
    {
      theta.push_back(atan((((vec[i].y - vec[(((i + 1)) % vec.size())].y)) / ((vec[i].x - vec[(((i + 1)) % vec.size())].x)))));
      i += 1;
    }
  }
  theta.push_back(0);
  theta.push_back((M_PI / 2));
  {
    var i = 0;
    while ((i < theta.size()))
    {
      var max_y = (-INF);
      var max_x = (-INF);
      var min_y = INF;
      var min_x = INF;
      {
        var j = 0;
        while ((j < vec.size()))
        {
          var x2 = ((vec[j].y * cos(theta[i])) - (vec[j].x * sin(theta[i])));
          var y2 = ((vec[j].y * sin(theta[i])) + (vec[j].x * cos(theta[i])));
          max_y = max(max_y, y2);
          max_x = max(max_x, x2);
          min_y = min(min_y, y2);
          min_x = min(min_x, x2);
          j += 1;
        }
      }
      ans = min(ans, (((max_y - min_y)) * ((max_x - min_x))));
      i += 1;
    }
  }
  printf("%.10lf\n", ans);
  return 0;
}
