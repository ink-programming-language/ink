// Translated from solution.cpp.

var MP = cpp_expression("#include");

var PB = cpp_expression("#include");

func ALL(s: dynamic)
{
  return cpp_expression("#include <iostream> #");
}

func EACH(i: dynamic, s: dynamic)
{
  cpp_macro("for (__typeof__((s).begin()) i = (s).begin(); i != (s).end(); ++i)");
}

func COUT(x: dynamic)
{
  return cpp_expression("#include <iostream> #include <sstream> #include <cstdio> #incl");
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func operator_shift_left(s: dynamic, P: dynamic)
{
  return (((((s << cpp_char("<")) << P.first) << ", ") << P.second) << cpp_char(">"));
}

func operator_shift_left(s: dynamic, P: dynamic)
{
  {
    var i = 0;
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

func operator_shift_left(s: dynamic, P: dynamic)
{
  {
    var i = 0;
    while ((i < P.size()))
    {
      ((s << endl) << P[i]);
      i += 1;
    }
  }
  return (s << endl);
}

func operator_shift_left(s: dynamic, P: dynamic)
{
  return s;
}

var INF = (1 << 60);

var EPS = 1e-10;

var PI = acos(-1.0);

func torad(deg: dynamic)
{
  return (((DD)(deg) * PI) / 180);
}

func todeg(ang: dynamic)
{
  return ((ang * 180) / PI);
}

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point(x: dynamic = 0.0, y: dynamic = 0.0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
}

func operator_add(p: dynamic, q: dynamic)
{
  return Point((p.x + q.x), (p.y + q.y));
}

func operator_subtract(p: dynamic, q: dynamic)
{
  return Point((p.x - q.x), (p.y - q.y));
}

func operator_multiply(p: dynamic, a: dynamic)
{
  return Point((p.x * a), (p.y * a));
}

func operator_multiply(a: dynamic, p: dynamic)
{
  return Point((a * p.x), (a * p.y));
}

func operator_multiply(p: dynamic, q: dynamic)
{
  return Point(((p.x * q.x) - (p.y * q.y)), ((p.x * q.y) + (p.y * q.x)));
}

func operator_divide(p: dynamic, a: dynamic)
{
  return Point((p.x / a), (p.y / a));
}

func conj(p: dynamic)
{
  return Point(p.x, (-p.y));
}

func rot(p: dynamic, ang: dynamic)
{
  return Point(((cos(ang) * p.x) - (sin(ang) * p.y)), ((sin(ang) * p.x) + (cos(ang) * p.y)));
}

func rot90(p: dynamic)
{
  return Point((-p.y), p.x);
}

func cross(p: dynamic, q: dynamic)
{
  return ((p.x * q.y) - (p.y * q.x));
}

func dot(p: dynamic, q: dynamic)
{
  return ((p.x * q.x) + (p.y * q.y));
}

func norm(p: dynamic)
{
  return dot(p, p);
}

func abs(p: dynamic)
{
  return sqrt(dot(p, p));
}

func amp(p: dynamic)
{
  var res = atan2(p.y, p.x);
  if ((res < 0))
  {
    res += (PI * 2);
  }
  return res;
}

func eq(p: dynamic, q: dynamic)
{
  return (abs((p - q)) < EPS);
}

func operator_less(p: dynamic, q: dynamic)
{
  return (if ((abs((p.x - q.x)) > EPS)) (p.x < q.x) else (p.y < q.y));
}

func operator_greater(p: dynamic, q: dynamic)
{
  return (if ((abs((p.x - q.x)) > EPS)) (p.x > q.x) else (p.y > q.y));
}

func operator_divide(p: dynamic, q: dynamic)
{
  return ((p * conj(q)) / norm(q));
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
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
  func Line(a: dynamic = Point(0.0, 0.0), b: dynamic = Point(0.0, 0.0))
  {
      this->push_back(a);
      this->push_back(b);
    }
}

class Circle
{
  var r: dynamic;
  func Circle(p: dynamic = Point(0.0, 0.0), r: dynamic = 0.0)
  {
      this->Point = cpp_construct(p);
      this->r = cpp_construct(r);
    }
}

func proj(p: dynamic, l: dynamic)
{
  var t = (dot((p - l[0]), (l[1] - l[0])) / norm((l[1] - l[0])));
  return (l[0] + (((l[1] - l[0])) * t));
}

func crosspoint(l: dynamic, m: dynamic)
{
  var res: dynamic;
  var d = cross((m[1] - m[0]), (l[1] - l[0]));
  if ((abs(d) < EPS))
  {
    return vector();
  }
  res.push_back((l[0] + ((((l[1] - l[0])) * cross((m[1] - m[0]), (m[1] - l[0]))) / d)));
  return res;
}

func crosspoint(e: dynamic, f: dynamic)
{
  var res: dynamic;
  var d = abs((e - f));
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
  var rcos = (((((d * d) + (e.r * e.r)) - (f.r * f.r))) / ((2.0 * d)));
  var rsin: dynamic;
  if (((e.r - abs(rcos)) < EPS))
  {
    rsin = 0;
  } else
  {
    rsin = sqrt(((e.r * e.r) - (rcos * rcos)));
  }
  var dir = (((f - e)) / d);
  var p1 = (e + (dir * Point(rcos, rsin)));
  var p2 = (e + (dir * Point(rcos, (-rsin))));
  res.push_back(p1);
  if ((!eq(p1, p2)))
  {
    res.push_back(p2);
  }
  return res;
}

func crosspoint(e: dynamic, l: dynamic)
{
  var res: dynamic;
  var p = proj(e, l);
  var rcos = abs((e - p));
  var rsin: dynamic;
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
  var dir = (((l[1] - l[0])) / abs((l[1] - l[0])));
  var p1 = (p + (dir * rsin));
  var p2 = (p - (dir * rsin));
  res.push_back(p1);
  if ((!eq(p1, p2)))
  {
    res.push_back(p2);
  }
  return res;
}

class Point3D
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  func Point3D(x: dynamic = 0.0, y: dynamic = 0.0, z: dynamic = 0.0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
      this->z = cpp_construct(z);
    }
}

func operator_add(p: dynamic, q: dynamic)
{
  return Point3D((p.x + q.x), (p.y + q.y), (p.z + q.z));
}

func operator_subtract(p: dynamic, q: dynamic)
{
  return Point3D((p.x - q.x), (p.y - q.y), (p.z - q.z));
}

func operator_multiply(p: dynamic, a: dynamic)
{
  return Point3D((p.x * a), (p.y * a), (p.z * a));
}

func operator_multiply(a: dynamic, p: dynamic)
{
  return Point3D((a * p.x), (a * p.y), (a * p.z));
}

func operator_multiply(p: dynamic, q: dynamic)
{
  return Point3D(((p.y * q.z) - (p.z * q.y)), ((p.z * q.x) - (p.x * q.z)), ((p.x * q.y) - (p.y * q.x)));
}

func operator_divide(p: dynamic, a: dynamic)
{
  return cpp_comma(Point3D((p.x / a), (p.y / a)), (p.z / a));
}

func dot(p: dynamic, q: dynamic)
{
  return (((p.x * q.x) + (p.y * q.y)) + (p.z * q.z));
}

func norm(p: dynamic)
{
  return dot(p, p);
}

func abs(p: dynamic)
{
  return sqrt(dot(p, p));
}

func eq(p: dynamic, q: dynamic)
{
  return (abs((p - q)) < EPS);
}

class Line3D
{
  func Line3D(a: dynamic = Point3D(0.0, 0.0, 0.0), b: dynamic = Point3D(0.0, 0.0, 0.0))
  {
      this->push_back(a);
      this->push_back(b);
    }
}

class Circle3D
{
  var r: dynamic;
  func Circle3D(p: dynamic = Point3D(0.0, 0.0), r: dynamic = 0.0)
  {
      this->Point3D = cpp_construct(p);
      this->r = cpp_construct(r);
    }
}

func proj(p: dynamic, l: dynamic)
{
  var t = (dot((p - l[0]), (l[1] - l[0])) / norm((l[1] - l[0])));
  return (l[0] + (((l[1] - l[0])) * t));
}

func proj(v: dynamic, d: dynamic)
{
  var t = (dot(v, d) / norm(d));
  return (v * t);
}

func refl(p: dynamic, l: dynamic)
{
  return (p + (((proj(p, l) - p)) * 2));
}

func isinterPL(p: dynamic, l: dynamic)
{
  return ((abs((p - proj(p, l))) < EPS));
}

func distancePL(p: dynamic, l: dynamic)
{
  return abs((p - proj(p, l)));
}

func distanceLL(l: dynamic, m: dynamic)
{
  var nv = (((l[1] - l[0])) * ((m[1] - m[0])));
  if ((abs(nv) < EPS))
  {
    return distancePL(l[0], m);
  }
  var p = (m[0] - l[0]);
  return (abs(dot(nv, p)) / abs(nv));
}

var X: dynamic;

var Y: dynamic;

var P: dynamic;

var r: dynamic;

func main()
{
  while (((((((((((cin >> X.x) >> X.y) >> X.z) >> Y.x) >> Y.y) >> Y.z) >> r) >> P.x) >> P.y) >> P.z))
  {
    var PH = proj(P, l);
    var x = cpp_construct(0, abs((X - Y)));
    var y = cpp_construct(0, 0);
    var p = cpp_construct(abs((P - PH)), abs((PH - Y)));
    var a = cpp_construct(r, 0);
    var b = cpp_construct((-r), 0);
    var vc = crosspoint(Line(p, a), Line(x, b));
    var c = vc[0];
    var vd = crosspoint(Line(p, b), Line(x, a));
    var d = vd[0];
    var m = (((c + d)) / 2);
    var h = proj(x, Line(c, d));
    var tsr = ((r * abs((x.y - m.y))) / abs((x - y)));
    var sr = sqrt(((tsr * tsr) - (m.x * m.x)));
    var tot = ((((PI * r) * r) * abs((x - y))) / 3);
    var sol = ((((PI * abs((c - d))) * sr) * abs((x - h))) / 6);
    write(fixed, setprecision(9), sol, " ", (tot - sol), "\n");
  }
  return 0;
}

func EACH(argument_0: dynamic, argument_1: dynamic)
{
    (((((s << "<") << it->first) << "->") << it->second) << "> ");
  }
