// Translated from solution.cpp.

func debug(x: dynamic) -> dynamic
{
  cpp_macro("cerr << #x << \" = \" << x << endl;");
}

var mod: dynamic = cpp_expression("#include <");

var INF: dynamic = cpp_expression("#include <");

var LLINF: dynamic = cpp_expression("#include <cstdio> #include <");

var SIZE: dynamic = cpp_expression("#inclu");

class P
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func P(X: dynamic, Y: dynamic) -> dynamic
  {
      x = X;
      y = Y;
    }
  func operator_less(B: dynamic) -> dynamic
  {
      return  ((x == B.x)) ? (y < B.y) : (x < B.x);
    }
}

func ccw(base: dynamic, A: dynamic, B: dynamic) -> dynamic
{
  return (((((A.x - base.x)) * ((B.y - base.y))) - (((A.y - base.y)) * ((B.x - base.x)))) >= 0);
}

func ConvexHull(s: dynamic) -> dynamic
{
  var g: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_cast(s.size());
  if ((n < 3))
  {
    return s;
  }
  sort(s.begin(), s.end());
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var m: dynamic = cpp_cast(g.size());
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
  var t: dynamic = cpp_cast(g.size());
  {
    var i: dynamic = (n - 2);
    while ((i >= 0))
    {
      {
        var m: dynamic = cpp_cast(g.size());
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_array(SIZE);
  var d: dynamic = cpp_array(SIZE);
  var y: dynamic = cpp_array(SIZE);
  var x: dynamic = cpp_array(SIZE);
  var s: dynamic = [];
  var vec: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  y[0] = cpp_assign(x[0], "=", 0);
  var dx: dynamic = [-1, 0, 1, 0];
  var dy: dynamic = [0, 1, 0, -1];
  {
    var i: dynamic = 1;
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
    var i: dynamic = 0;
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
  var ans: dynamic = LLINF;
  var theta: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < vec.size()))
    {
      theta.push_back(atan((((vec[i].y - vec[(((i + 1)) % vec.size())].y)) / ((vec[i].x - vec[(((i + 1)) % vec.size())].x)))));
      i += 1;
    }
  }
  theta.push_back(0);
  theta.push_back((M_PI / 2));
  {
    var i: dynamic = 0;
    while ((i < theta.size()))
    {
      var max_y: dynamic = (-INF);
      var max_x: dynamic = (-INF);
      var min_y: dynamic = INF;
      var min_x: dynamic = INF;
      {
        var j: dynamic = 0;
        while ((j < vec.size()))
        {
          var x2: dynamic = ((vec[j].y * cos(theta[i])) - (vec[j].x * sin(theta[i])));
          var y2: dynamic = ((vec[j].y * sin(theta[i])) + (vec[j].x * cos(theta[i])));
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
