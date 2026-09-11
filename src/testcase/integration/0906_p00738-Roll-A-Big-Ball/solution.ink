// Translated from solution.cpp.

var EPS: dynamic = 1e-8;

var INF: dynamic = 1e12;

var X: dynamic = cpp_expression("#inc");

var Y: dynamic = cpp_expression("#inc");

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  ((real(a) != real(b))) ? (real(a) < real(b)) : (imag(a) < imag(b));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return imag((conj(a) * b));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return real((conj(a) * b));
}

class L
{
  func L() -> dynamic
  {
    }
  func L(a: dynamic, b: dynamic) -> dynamic
  {
      push_back(a);
      push_back(b);
    }
}

class C
{
  var p: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func C(p: dynamic, r: dynamic) -> dynamic
  {
      self->p = cpp_construct(p);
      self->r = cpp_construct(r);
    }
  func R(h: dynamic) -> dynamic
  {
      return sqrt(max(((r * r) - (h * h)), 0.0));
    }
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  if ((cross(b, c) > 0))
  {
    return +1;
  }
  if ((cross(b, c) < 0))
  {
    return -1;
  }
  if ((dot(b, c) < 0))
  {
    return +2;
  }
  if ((norm(b) < norm(c)))
  {
    return -2;
  }
  return 0;
}

func projection(l: dynamic, p: dynamic) -> dynamic
{
  var t: dynamic = (dot((p - l[0]), (l[0] - l[1])) / norm((l[0] - l[1])));
  return (l[0] + (t * ((l[0] - l[1]))));
}

func distanceLP(l: dynamic, p: dynamic) -> dynamic
{
  return abs((p - projection(l, p)));
}

func intersectSP(s: dynamic, p: dynamic) -> dynamic
{
  return (((abs((s[0] - p)) + abs((s[1] - p))) - abs((s[1] - s[0]))) < EPS);
}

func distanceSP(s: dynamic, p: dynamic) -> dynamic
{
  var r: dynamic = projection(s, p);
  if (intersectSP(s, r))
  {
    return abs((r - p));
  }
  return min(abs((s[0] - p)), abs((s[1] - p)));
}

func intersectSS(s: dynamic, t: dynamic) -> dynamic
{
  return (((ccw(s[0], s[1], t[0]) * ccw(s[0], s[1], t[1])) <= 0) && ((ccw(t[0], t[1], s[0]) * ccw(t[0], t[1], s[1])) <= 0));
}

class B
{
  var x1: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  func inter(l: dynamic) -> dynamic
  {
      return ((((((x1 <= l[0].X()) && (l[0].X() <= x2))) && (((y1 <= l[0].Y()) && (l[0].Y() <= y2))))) || (((((x1 <= l[1].X()) && (l[1].X() <= x2))) && (((y1 <= l[1].Y()) && (l[1].Y() <= y2))))));
    }
}

var line: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  line.clear();
  b.clear();
}

func input() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  if ((n == 0))
  {
    return false;
  }
  var tmp: dynamic = cpp_array(4);
  read(tmp[0], tmp[1], tmp[2], tmp[3]);
  line = L(P(tmp[0], tmp[1]), P(tmp[2], tmp[3]));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var h: dynamic = cpp_uninitialized();
      read(tmp[0], tmp[1], tmp[2], tmp[3], h);
      b.push_back([tmp[0], tmp[1], tmp[2], tmp[3], h]);
      i += 1;
    }
  }
  return true;
}

func getR(dist: dynamic, h: dynamic) -> dynamic
{
  if ((dist < h))
  {
    return dist;
  }
  var res: dynamic = (((((dist * dist) - (h * h))) / ((2 * h))) + h);
  return res;
}

func notouchR(b: dynamic) -> dynamic
{
  var bl: dynamic = [L(P(b.x1, b.y1), P(b.x1, b.y2)), L(P(b.x1, b.y2), P(b.x2, b.y2)), L(P(b.x2, b.y2), P(b.x2, b.y1)), L(P(b.x2, b.y1), P(b.x1, b.y1))];
  var bp: dynamic = [P(b.x1, b.y1), P(b.x1, b.y2), P(b.x2, b.y1), P(b.x2, b.y2)];
  if (b.inter(line))
  {
    return 0;
  }
  for (var i: dynamic in bl)
  {
    if (intersectSS(line, i))
    {
      return 0;
    }
  }
  var dist: dynamic = 1000000;
  for (var i: dynamic in bl)
  {
    dist = min(dist, distanceSP(i, line[0]));
    dist = min(dist, distanceSP(i, line[1]));
  }
  for (var i: dynamic in bp)
  {
    dist = min(dist, distanceSP(line, i));
  }
  return getR(dist, b.h);
}

func solve() -> dynamic
{
  var ans: dynamic = 10000;
  {
    var i: dynamic = 0;
    while ((i < b.size()))
    {
      ans = min(notouchR(b[i]), ans);
      i += 1;
    }
  }
  return ans;
}

func main() -> dynamic
{
  while (cpp_comma(init(), input()))
  {
    write(fixed, setprecision(20), solve(), "\n");
  }
}
