// Translated from solution.cpp.

var X: dynamic = cpp_expression("#inclu");

var Y: dynamic = cpp_expression("#inclu");

var EPS: dynamic = (1e-10);

var INF: dynamic = (1e15);

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a.X != b.X)) ? (a.X < b.X) : (a.Y < b.Y);
}

func cmp_y(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a.Y != b.Y)) ? (a.Y < b.Y) : (a.X < b.X);
}

func operator_equal(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a - b)) < EPS);
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.X * b.X) + (a.Y * b.Y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.X * b.Y) - (a.Y * b.X));
}

func intersection(a: dynamic, b: dynamic) -> dynamic
{
  var af: dynamic = a.first;
  var as_cpp: dynamic = a.second;
  var bf: dynamic = b.first;
  var bs: dynamic = b.second;
  return (af + ((cross((bs - bf), (af - bf)) / (((-cross((bs - bf), (as_cpp - bf))) + cross((bs - bf), (af - bf))))) * ((as_cpp - af))));
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  if ((cross(b, c) > EPS))
  {
    return +1;
  }
  if ((cross(b, c) < (-EPS)))
  {
    return -1;
  }
  if ((dot(b, c) < 0))
  {
    return +2;
  }
  if (((abs(b) + EPS) < abs(c)))
  {
    return -2;
  }
  return 0;
}

func convex_cut(p: dynamic, l: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var n: dynamic = p.size();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((ccw(l.first, l.second, p[i]) != -1))
      {
        ret.push_back(p[i]);
      }
      if (((ccw(l.first, l.second, p[i]) != -1) ^ (ccw(l.first, l.second, p[(((i + 1)) % n)]) != -1)))
      {
        ret.push_back(intersection(L(p[i], p[(((i + 1)) % n)]), l));
      }
      i += 1;
    }
  }
  return ret;
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var Po: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      Po.emplace_back(x, y);
      i += 1;
    }
  }
  var M: dynamic = cpp_uninitialized();
  read(M);
  var C: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      C.emplace_back(x, y);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var Q: dynamic = Po;
      {
        var j: dynamic = 0;
        while ((j < M))
        {
          if ((j == i))
          {
            j += 1;
            continue;
          }
          var m: dynamic = (((C[i] + C[j])) / 2.0);
          Q = convex_cut(Q, L(m, (m + (((C[j] - C[i])) * P(0, 1)))));
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < Q.size()))
        {
          ans = max(ans, abs((Q[j] - C[i])));
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%.12f\n", ans);
  return 0;
}
