// Translated from solution.cpp.

var EPS = 1e-8;

var INF = 1e12;

var X = cpp_expression("#inc");

var Y = cpp_expression("#inc");

func operator_less(a: dynamic, b: dynamic)
{
  return if ((real(a) != real(b))) (real(a) < real(b)) else (imag(a) < imag(b));
}

func cross(a: dynamic, b: dynamic)
{
  return imag((conj(a) * b));
}

func dot(a: dynamic, b: dynamic)
{
  return real((conj(a) * b));
}

class L
{
  func L()
  {
    }
  func L(a: dynamic, b: dynamic)
  {
      push_back(a);
      push_back(b);
    }
}

class C
{
  var p: dynamic;
  var r: dynamic;
  func C(p: dynamic, r: dynamic)
  {
      this->p = cpp_construct(p);
      this->r = cpp_construct(r);
    }
  func R(h: dynamic)
  {
      return sqrt(max(((r * r) - (h * h)), 0.0));
    }
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
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

func projection(l: dynamic, p: dynamic)
{
  var t = (dot((p - l[0]), (l[0] - l[1])) / norm((l[0] - l[1])));
  return (l[0] + (t * ((l[0] - l[1]))));
}

func distanceLP(l: dynamic, p: dynamic)
{
  return abs((p - projection(l, p)));
}

func intersectSP(s: dynamic, p: dynamic)
{
  return (((abs((s[0] - p)) + abs((s[1] - p))) - abs((s[1] - s[0]))) < EPS);
}

func distanceSP(s: dynamic, p: dynamic)
{
  var r = projection(s, p);
  if (intersectSP(s, r))
  {
    return abs((r - p));
  }
  return min(abs((s[0] - p)), abs((s[1] - p)));
}

func intersectSS(s: dynamic, t: dynamic)
{
  return (((ccw(s[0], s[1], t[0]) * ccw(s[0], s[1], t[1])) <= 0) && ((ccw(t[0], t[1], s[0]) * ccw(t[0], t[1], s[1])) <= 0));
}

class B
{
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  var h: dynamic;
  func inter(l: dynamic)
  {
      return ((((((x1 <= l[0].X()) && (l[0].X() <= x2))) && (((y1 <= l[0].Y()) && (l[0].Y() <= y2))))) || (((((x1 <= l[1].X()) && (l[1].X() <= x2))) && (((y1 <= l[1].Y()) && (l[1].Y() <= y2))))));
    }
}

var line: dynamic;

var b: dynamic;

func init()
{
  line.clear();
  b.clear();
}

func input()
{
  var n: dynamic;
  read(n);
  if ((n == 0))
  {
    return false;
  }
  var tmp = cpp_array(4);
  read(tmp[0], tmp[1], tmp[2], tmp[3]);
  line = L(P(tmp[0], tmp[1]), P(tmp[2], tmp[3]));
  {
    var i = 0;
    while ((i < n))
    {
      var h: dynamic;
      read(tmp[0], tmp[1], tmp[2], tmp[3], h);
      b.push_back([tmp[0], tmp[1], tmp[2], tmp[3], h]);
      i += 1;
    }
  }
  return true;
}

func getR(dist: dynamic, h: dynamic)
{
  if ((dist < h))
  {
    return dist;
  }
  var res = (((((dist * dist) - (h * h))) / ((2 * h))) + h);
  return res;
}

func notouchR(b: dynamic)
{
  var bl = [L(P(b.x1, b.y1), P(b.x1, b.y2)), L(P(b.x1, b.y2), P(b.x2, b.y2)), L(P(b.x2, b.y2), P(b.x2, b.y1)), L(P(b.x2, b.y1), P(b.x1, b.y1))];
  var bp = [P(b.x1, b.y1), P(b.x1, b.y2), P(b.x2, b.y1), P(b.x2, b.y2)];
  if (b.inter(line))
  {
    return 0;
  }
  for (var i in bl)
  {
    if (intersectSS(line, i))
    {
      return 0;
    }
  }
  var dist = 1000000;
  for (var i in bl)
  {
    dist = min(dist, distanceSP(i, line[0]));
    dist = min(dist, distanceSP(i, line[1]));
  }
  for (var i in bp)
  {
    dist = min(dist, distanceSP(line, i));
  }
  return getR(dist, b.h);
}

func solve()
{
  var ans = 10000;
  {
    var i = 0;
    while ((i < b.size()))
    {
      ans = min(notouchR(b[i]), ans);
      i += 1;
    }
  }
  return ans;
}

func main()
{
  while (cpp_comma(init(), input()))
  {
    write(fixed, setprecision(20), solve(), "\n");
  }
}
