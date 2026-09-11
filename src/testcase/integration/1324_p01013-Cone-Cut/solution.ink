// Translated from solution.cpp.

var MP: dynamic = cpp_expression("#include");

var PB: dynamic = cpp_expression("#include");

func ALL(s: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #");
}

func EACH(i: dynamic, s: dynamic) -> dynamic
{
  cpp_macro("for (__typeof__((s).begin()) i = (s).begin(); i != (s).end(); ++i)");
}

func COUT(x: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #include <sstream> #include <cstdio> #incl");
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func operator_shift_left(s: dynamic, P: dynamic) -> dynamic
{
  return (((((s << cpp_char("<")) << P.first) << ", ") << P.second) << cpp_char(">"));
}

func operator_shift_left(s: dynamic, P: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < P.size()))
    {
      if ((i > 0))
      {
        (s << " ");
      }
      (s << P[i]);
      i += 1;
    }
  }
  return s;
}

func operator_shift_left(s: dynamic, P: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < P.size()))
    {
      ((s << endl) << P[i]);
      i += 1;
    }
  }
  return (s << endl);
}

func operator_shift_left(s: dynamic, P: dynamic) -> dynamic
{
  return s;
}

var INF: dynamic = (1 << 60);

var EPS: dynamic = 1e-10;

var PI: dynamic = acos(-1.0);

func torad(deg: dynamic) -> dynamic
{
  return (((DD)(deg) * PI) / 180);
}

func todeg(ang: dynamic) -> dynamic
{
  return ((ang * 180) / PI);
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = 0.0, y: dynamic = 0.0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
}

func operator_add(p: dynamic, q: dynamic) -> dynamic
{
  return Point((p.x + q.x), (p.y + q.y));
}

func operator_subtract(p: dynamic, q: dynamic) -> dynamic
{
  return Point((p.x - q.x), (p.y - q.y));
}

func operator_multiply(p: dynamic, a: dynamic) -> dynamic
{
  return Point((p.x * a), (p.y * a));
}

func operator_multiply(a: dynamic, p: dynamic) -> dynamic
{
  return Point((a * p.x), (a * p.y));
}

func operator_multiply(p: dynamic, q: dynamic) -> dynamic
{
  return Point(((p.x * q.x) - (p.y * q.y)), ((p.x * q.y) + (p.y * q.x)));
}

func operator_divide(p: dynamic, a: dynamic) -> dynamic
{
  return Point((p.x / a), (p.y / a));
}

func conj(p: dynamic) -> dynamic
{
  return Point(p.x, (-p.y));
}

func rot(p: dynamic, ang: dynamic) -> dynamic
{
  return Point(((cos(ang) * p.x) - (sin(ang) * p.y)), ((sin(ang) * p.x) + (cos(ang) * p.y)));
}

func rot90(p: dynamic) -> dynamic
{
  return Point((-p.y), p.x);
}

func cross(p: dynamic, q: dynamic) -> dynamic
{
  return ((p.x * q.y) - (p.y * q.x));
}

func dot(p: dynamic, q: dynamic) -> dynamic
{
  return ((p.x * q.x) + (p.y * q.y));
}

func norm(p: dynamic) -> dynamic
{
  return dot(p, p);
}

func abs(p: dynamic) -> dynamic
{
  return sqrt(dot(p, p));
}

func amp(p: dynamic) -> dynamic
{
  var res: dynamic = atan2(p.y, p.x);
  if ((res < 0))
  {
    res += (PI * 2);
  }
  return res;
}

func eq(p: dynamic, q: dynamic) -> dynamic
{
  return (abs((p - q)) < EPS);
}

func operator_less(p: dynamic, q: dynamic) -> dynamic
{
  return ( ((abs((p.x - q.x)) > EPS)) ? (p.x < q.x) : (p.y < q.y));
}

func operator_greater(p: dynamic, q: dynamic) -> dynamic
{
  return ( ((abs((p.x - q.x)) > EPS)) ? (p.x > q.x) : (p.y > q.y));
}

func operator_divide(p: dynamic, q: dynamic) -> dynamic
{
  return ((p * conj(q)) / norm(q));
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  if ((cross((b - a), (c - a)) > EPS))
  {
    return 1;
  }
  if ((cross((b - a), (c - a)) < (-EPS)))
  {
    return -1;
  }
  if ((dot((b - a), (c - a)) < (-EPS)))
  {
    return 2;
  }
  if ((norm((b - a)) < (norm((c - a)) - EPS)))
  {
    return -2;
  }
  return 0;
}

class Line
{
  func Line(a: dynamic = Point(0.0, 0.0), b: dynamic = Point(0.0, 0.0)) -> dynamic
  {
      self->push_back(a);
      self->push_back(b);
    }
}

class Circle
{
  var r: dynamic = cpp_uninitialized();
  func Circle(p: dynamic = Point(0.0, 0.0), r: dynamic = 0.0) -> dynamic
  {
      self->Point = cpp_construct(p);
      self->r = cpp_construct(r);
    }
}

func proj(p: dynamic, l: dynamic) -> dynamic
{
  var t: dynamic = (dot((p - l[0]), (l[1] - l[0])) / norm((l[1] - l[0])));
  return (l[0] + (((l[1] - l[0])) * t));
}

func crosspoint(l: dynamic, m: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  var d: dynamic = cross((m[1] - m[0]), (l[1] - l[0]));
  if ((abs(d) < EPS))
  {
    return vector();
  }
  res.push_back((l[0] + ((((l[1] - l[0])) * cross((m[1] - m[0]), (m[1] - l[0]))) / d)));
  return res;
}

func crosspoint(e: dynamic, f: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  var d: dynamic = abs((e - f));
  if ((d < EPS))
  {
    return vector();
  }
  if ((d > ((e.r + f.r) + EPS)))
  {
    return vector();
  }
  if ((d < (abs((e.r - f.r)) - EPS)))
  {
    return vector();
  }
  var rcos: dynamic = (((((d * d) + (e.r * e.r)) - (f.r * f.r))) / ((2.0 * d)));
  var rsin: dynamic = cpp_uninitialized();
  if (((e.r - abs(rcos)) < EPS))
  {
    rsin = 0;
  } else
  {
    rsin = sqrt(((e.r * e.r) - (rcos * rcos)));
  }
  var dir: dynamic = (((f - e)) / d);
  var p1: dynamic = (e + (dir * Point(rcos, rsin)));
  var p2: dynamic = (e + (dir * Point(rcos, (-rsin))));
  res.push_back(p1);
  if ((!eq(p1, p2)))
  {
    res.push_back(p2);
  }
  return res;
}

func crosspoint(e: dynamic, l: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  var p: dynamic = proj(e, l);
  var rcos: dynamic = abs((e - p));
  var rsin: dynamic = cpp_uninitialized();
  if ((rcos > (e.r + EPS)))
  {
    return vector();
  } else if (((e.r - rcos) < EPS))
  {
    rsin = 0;
  } else
  {
    rsin = sqrt(((e.r * e.r) - (rcos * rcos)));
  }
  var dir: dynamic = (((l[1] - l[0])) / abs((l[1] - l[0])));
  var p1: dynamic = (p + (dir * rsin));
  var p2: dynamic = (p - (dir * rsin));
  res.push_back(p1);
  if ((!eq(p1, p2)))
  {
    res.push_back(p2);
  }
  return res;
}

class Point3D
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  func Point3D(x: dynamic = 0.0, y: dynamic = 0.0, z: dynamic = 0.0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
      self->z = cpp_construct(z);
    }
}

func operator_add(p: dynamic, q: dynamic) -> dynamic
{
  return Point3D((p.x + q.x), (p.y + q.y), (p.z + q.z));
}

func operator_subtract(p: dynamic, q: dynamic) -> dynamic
{
  return Point3D((p.x - q.x), (p.y - q.y), (p.z - q.z));
}

func operator_multiply(p: dynamic, a: dynamic) -> dynamic
{
  return Point3D((p.x * a), (p.y * a), (p.z * a));
}

func operator_multiply(a: dynamic, p: dynamic) -> dynamic
{
  return Point3D((a * p.x), (a * p.y), (a * p.z));
}

func operator_multiply(p: dynamic, q: dynamic) -> dynamic
{
  return Point3D(((p.y * q.z) - (p.z * q.y)), ((p.z * q.x) - (p.x * q.z)), ((p.x * q.y) - (p.y * q.x)));
}

func operator_divide(p: dynamic, a: dynamic) -> dynamic
{
  return cpp_comma(Point3D((p.x / a), (p.y / a)), (p.z / a));
}

func dot(p: dynamic, q: dynamic) -> dynamic
{
  return (((p.x * q.x) + (p.y * q.y)) + (p.z * q.z));
}

func norm(p: dynamic) -> dynamic
{
  return dot(p, p);
}

func abs(p: dynamic) -> dynamic
{
  return sqrt(dot(p, p));
}

func eq(p: dynamic, q: dynamic) -> dynamic
{
  return (abs((p - q)) < EPS);
}

class Line3D
{
  func Line3D(a: dynamic = Point3D(0.0, 0.0, 0.0), b: dynamic = Point3D(0.0, 0.0, 0.0)) -> dynamic
  {
      self->push_back(a);
      self->push_back(b);
    }
}

class Circle3D
{
  var r: dynamic = cpp_uninitialized();
  func Circle3D(p: dynamic = Point3D(0.0, 0.0), r: dynamic = 0.0) -> dynamic
  {
      self->Point3D = cpp_construct(p);
      self->r = cpp_construct(r);
    }
}

func proj(p: dynamic, l: dynamic) -> dynamic
{
  var t: dynamic = (dot((p - l[0]), (l[1] - l[0])) / norm((l[1] - l[0])));
  return (l[0] + (((l[1] - l[0])) * t));
}

func proj(v: dynamic, d: dynamic) -> dynamic
{
  var t: dynamic = (dot(v, d) / norm(d));
  return (v * t);
}

func refl(p: dynamic, l: dynamic) -> dynamic
{
  return (p + (((proj(p, l) - p)) * 2));
}

func isinterPL(p: dynamic, l: dynamic) -> dynamic
{
  return ((abs((p - proj(p, l))) < EPS));
}

func distancePL(p: dynamic, l: dynamic) -> dynamic
{
  return abs((p - proj(p, l)));
}

func distanceLL(l: dynamic, m: dynamic) -> dynamic
{
  var nv: dynamic = (((l[1] - l[0])) * ((m[1] - m[0])));
  if ((abs(nv) < EPS))
  {
    return distancePL(l[0], m);
  }
  var p: dynamic = (m[0] - l[0]);
  return (abs(dot(nv, p)) / abs(nv));
}

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

var P: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while (((((((((((cin >> X.x) >> X.y) >> X.z) >> Y.x) >> Y.y) >> Y.z) >> r) >> P.x) >> P.y) >> P.z))
  {
    var PH: dynamic = proj(P, l);
    var x: dynamic = cpp_construct(0, abs((X - Y)));
    var y: dynamic = cpp_construct(0, 0);
    var p: dynamic = cpp_construct(abs((P - PH)), abs((PH - Y)));
    var a: dynamic = cpp_construct(r, 0);
    var b: dynamic = cpp_construct((-r), 0);
    var vc: dynamic = crosspoint(Line(p, a), Line(x, b));
    var c: dynamic = vc[0];
    var vd: dynamic = crosspoint(Line(p, b), Line(x, a));
    var d: dynamic = vd[0];
    var m: dynamic = (((c + d)) / 2);
    var h: dynamic = proj(x, Line(c, d));
    var tsr: dynamic = ((r * abs((x.y - m.y))) / abs((x - y)));
    var sr: dynamic = sqrt(((tsr * tsr) - (m.x * m.x)));
    var tot: dynamic = ((((PI * r) * r) * abs((x - y))) / 3);
    var sol: dynamic = ((((PI * abs((c - d))) * sr) * abs((x - h))) / 6);
    write(fixed, setprecision(9), sol, " ", (tot - sol), "\n");
  }
  return 0;
}

func EACH(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    (((((s << "<") << it->first) << "->") << it->second) << "> ");
  }
