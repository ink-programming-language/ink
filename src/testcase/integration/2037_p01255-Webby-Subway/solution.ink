// Translated from solution.cpp.

func ALL(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/std");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func debug(v: dynamic) -> dynamic
{
  cpp_macro("cout<<#v<<\":\";for(auto x:v){cout<<x<<' ';}cout<<endl;");
}

var INF: dynamic = cpp_expression("#include<b");

var mod: dynamic = cpp_expression("#include<b");

var LINF: dynamic = 1001002003004005006;

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
    return true;
  }
  return false;
}

var EPS: dynamic = 1e-10;

var pi: dynamic = acosl(-1);

func operator_shift_right(is: dynamic, p: dynamic) -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  ((is >> a) >> b);
  p = Point(a, b);
  return is;
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << fixed) << setprecision(12)) << p.real()) << cpp_char(" ")) << p.imag());
}

func eq(a: dynamic, b: dynamic) -> dynamic
{
  return (fabs((a - b)) < EPS);
}

func operator_multiply(p: dynamic, d: dynamic) -> dynamic
{
  return Point((real(p) * d), (imag(p) * d));
}

class Line
{
  var p1: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
  func Line() -> dynamic
  {
    }
  func Line(p1: dynamic, p2: dynamic) -> dynamic
  {
      self->p1 = cpp_construct(p1);
      self->p2 = cpp_construct(p2);
    }
  func Line(A: dynamic, B: dynamic, C: dynamic) -> dynamic
  {
      if (eq(A, 0))
      {
        p1 = Point(0, (C / B));
        p2 = Point(1, (C / B));
      } else if (eq(B, 0))
      {
        p1 = Point((C / A), 0);
        p2 = Point((C / A), 1);
      } else
      {
        p1 = Point(0, (C / B));
        p2 = Point((C / A), 0);
      }
    }
}

class Segment
{
  func Segment() -> dynamic
  {
    }
  func Segment(p1: dynamic, p2: dynamic) -> dynamic
  {
      self->Line = cpp_construct(p1, p2);
    }
}

class Circle
{
  var center: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func Circle() -> dynamic
  {
    }
  func Circle(center: dynamic, r: dynamic) -> dynamic
  {
      self->center = cpp_construct(center);
      self->r = cpp_construct(r);
    }
}

func rotate(theta: dynamic, p: dynamic) -> dynamic
{
  return Point(((cos(theta) * p.real()) - (sin(theta) * p.imag())), ((sin(theta) * p.real()) + (cos(theta) * p.imag())));
}

func radian_to_degree(r: dynamic) -> dynamic
{
  return ((r * 180.0) / pi);
}

func degree_to_radian(d: dynamic) -> dynamic
{
  return ((d * pi) / 180.0);
}

func area_triangle(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var x: dynamic = (b - a);
  var y: dynamic = (c - a);
  return (fabs(((x.real() * y.imag()) - (x.imag() * y.real()))) / 2);
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((real(a) * imag(b)) - (imag(a) * real(b)));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((real(a) * real(b)) + (imag(a) * imag(b)));
}

func parallel(a: dynamic, b: dynamic) -> dynamic
{
  return eq(cross((a.p1 - a.p2), (b.p1 - b.p2)), 0.0);
}

func orthogonal(a: dynamic, b: dynamic) -> dynamic
{
  return eq(dot((a.p1 - a.p2), (b.p1 - b.p2)), 0.0);
}

func projection(l: dynamic, p: dynamic) -> dynamic
{
  var k: dynamic = (dot((l.p1 - l.p2), (p - l.p1)) / norm((l.p1 - l.p2)));
  return (l.p1 + (((l.p1 - l.p2)) * k));
}

func projection(l: dynamic, p: dynamic) -> dynamic
{
  var k: dynamic = (dot((l.p1 - l.p2), (p - l.p1)) / norm((l.p1 - l.p2)));
  return (l.p1 + (((l.p1 - l.p2)) * k));
}

func reflection(l: dynamic, p: dynamic) -> dynamic
{
  var h: dynamic = projection(l, p);
  return (((p + ((h - p))) + ((h - p))));
}

func reflection(l: dynamic, p: dynamic) -> dynamic
{
  var h: dynamic = projection(l, p);
  return (((p + ((h - p))) + ((h - p))));
}

func dis(a: dynamic, b: dynamic) -> dynamic
{
  return abs((a - b));
}

func dis(l: dynamic, p: dynamic) -> dynamic
{
  return abs((p - projection(l, p)));
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  if ((cross(b, c) > EPS))
  {
    return 1;
  } else if ((cross(b, c) < (-EPS)))
  {
    return -1;
  } else if ((dot(b, c) < 0))
  {
    return 2;
  } else if ((norm(b) < norm(c)))
  {
    return -2;
  } else
  {
    return 0;
  }
}

func circumcenter(A: dynamic, B: dynamic, C: dynamic) -> dynamic
{
  var S: dynamic = area_triangle(A, B, C);
  var a: dynamic = dis(B, C);
  var b: dynamic = dis(A, C);
  var c: dynamic = dis(A, B);
  return (((A * ((((a * a) * ((((b * b) + (c * c)) - (a * a)))) / (((16 * S) * S))))) + (B * ((((b * b) * ((((c * c) + (a * a)) - (b * b)))) / (((16 * S) * S)))))) + (C * ((((c * c) * ((((a * a) + (b * b)) - (c * c)))) / (((16 * S) * S))))));
}

func intersect(l: dynamic, p: dynamic) -> dynamic
{
  return (abs(ccw(l.p1, l.p2, p)) != 1);
}

func intersect(l1: dynamic, l2: dynamic) -> dynamic
{
  return cpp_binary((abs(cross((l1.p2 - l1.p1), (l2.p2 - l2.p1))) > EPS), "or", (abs(cross((l1.p2 - l1.p1), (l2.p2 - l1.p1))) < EPS));
}

func intersect(s: dynamic, p: dynamic) -> dynamic
{
  return (ccw(s.p1, s.p2, p) == 0);
}

func intersect(l: dynamic, s: dynamic) -> dynamic
{
  return ((cross((l.p2 - l.p1), (s.p1 - l.p1)) * cross((l.p2 - l.p1), (s.p2 - l.p1))) < EPS);
}

func intersect(c: dynamic, l: dynamic) -> dynamic
{
  return (dis(l, c.center) <= (c.r + EPS));
}

func intersect(c: dynamic, p: dynamic) -> dynamic
{
  return (abs((abs((p - c.center)) - c.r)) < EPS);
}

func intersect(s: dynamic, t: dynamic) -> dynamic
{
  return cpp_binary(((ccw(s.p1, s.p2, t.p1) * ccw(s.p1, s.p2, t.p2)) <= 0), "and", ((ccw(t.p1, t.p2, s.p1) * ccw(t.p1, t.p2, s.p2)) <= 0));
}

func intersect(c: dynamic, l: dynamic) -> dynamic
{
  var h: dynamic = projection(l, c.center);
  if (((norm((h - c.center)) - (c.r * c.r)) > EPS))
  {
    return 0;
  }
  var d1: dynamic = abs((c.center - l.p1));
  var d2: dynamic = abs((c.center - l.p2));
  if (cpp_binary((d1 < (c.r + EPS)), "and", (d2 < (c.r + EPS))))
  {
    return 0;
  }
  if (cpp_binary((cpp_binary((d1 < (c.r - EPS)), "and", (d2 > (c.r + EPS)))), "or", (cpp_binary((d2 < (c.r - EPS)), "and", (d1 > (c.r + EPS))))))
  {
    return 1;
  }
  if ((dot((l.p1 - h), (l.p2 - h)) < 0))
  {
    return 2;
  }
  return 0;
}

func intersect(c1: dynamic, c2: dynamic) -> dynamic
{
  if ((c1.r < c2.r))
  {
    swap(c1, c2);
  }
  var d: dynamic = abs((c1.center - c2.center));
  if (((c1.r + c2.r) < d))
  {
    return 4;
  }
  if (eq((c1.r + c2.r), d))
  {
    return 3;
  }
  if (((c1.r - c2.r) < d))
  {
    return 2;
  }
  if (eq((c1.r - c2.r), d))
  {
    return 1;
  }
  return 0;
}

func crosspoint(l: dynamic, m: dynamic) -> dynamic
{
  var A: dynamic = cross((m.p2 - m.p1), (m.p1 - l.p1));
  var B: dynamic = cross((m.p2 - m.p1), (l.p2 - l.p1));
  if (cpp_binary(eq(A, 0), "and", eq(B, 0)))
  {
    return l.p1;
  }
  if (eq(B, 0))
  {
    return null;
  }
  return (l.p1 + ((A / B) * ((l.p2 - l.p1))));
}

func crosspoint(l: dynamic, m: dynamic) -> dynamic
{
  return crosspoint(Line(l), Line(m));
}

func crosspoint(c: dynamic, l: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var h: dynamic = projection(l, c.center);
  var d: dynamic = sqrt(((c.r * c.r) - norm((h - c.center))));
  var e: dynamic = (((l.p2 - l.p1)) * ((1 / abs((l.p2 - l.p1)))));
  if ((((c.r * c.r) + EPS) < norm((h - c.center))))
  {
    return ret;
  }
  if (eq(dis(l, c.center), c.r))
  {
    ret.push_back(h);
    return ret;
  }
  ret.push_back((h + (e * d)));
  ret.push_back((h - (e * d)));
  return ret;
}

func crosspoint(c: dynamic, s: dynamic) -> dynamic
{
  var l: dynamic = Line(s.p1, s.p2);
  var ko: dynamic = intersect(c, s);
  if ((ko == 2))
  {
    return crosspoint(c, l);
  }
  var ret: dynamic = cpp_uninitialized();
  if ((ko == 0))
  {
    return ret;
  }
  ret = crosspoint(c, l);
  if ((ret.size() == 1))
  {
    return ret;
  }
  var rret: dynamic = cpp_uninitialized();
  if ((dot((s.p1 - ret[0]), (s.p2 - ret[0])) < 0))
  {
    rret.push_back(ret[0]);
  } else
  {
    rret.push_back(ret[1]);
  }
  return rret;
}

func crosspoint(c1: dynamic, c2: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var isec: dynamic = intersect(c1, c2);
  if (cpp_binary((isec == 0), "or", (isec == 4)))
  {
    return ret;
  }
  var d: dynamic = abs((c1.center - c2.center));
  var a: dynamic = acos((((((c1.r * c1.r) + (d * d)) - (c2.r * c2.r))) / (((2 * c1.r) * d))));
  var t: dynamic = atan2((c2.center.imag() - c1.center.imag()), (c2.center.real() - c1.center.real()));
  ret.push_back((c1.center + Point((cos((t + a)) * c1.r), (sin((t + a)) * c1.r))));
  ret.push_back((c1.center + Point((cos((t - a)) * c1.r), (sin((t - a)) * c1.r))));
  return ret;
}

func tangent(c: dynamic, p: dynamic) -> dynamic
{
  return crosspoint(c, Circle(p, sqrt((norm((c.center - p)) - (c.r * c.r)))));
}

func tangent(c1: dynamic, c2: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  if ((c1.r < c2.r))
  {
    swap(c1, c2);
  }
  var g: dynamic = norm((c1.center - c2.center));
  if (eq(g, 0))
  {
    return ret;
  }
  var u: dynamic = (((c2.center - c1.center)) / sqrt(g));
  var v: dynamic = rotate((pi * 0.5), u);
  for (var s: dynamic in [-1, 1])
  {
    var h: dynamic = (((c1.r + (s * c2.r))) / sqrt(g));
    if (eq((1 - (h * h)), 0))
    {
      ret.push_back(Line((c1.center + (u * c1.r)), (c1.center + (((u + v)) * c1.r))));
    } else if (((1 - (h * h)) > 0))
    {
      var uu: dynamic = (u * h);
      var vv: dynamic = (v * sqrt((1 - (h * h))));
      ret.push_back(Line((c1.center + (((uu + vv)) * c1.r)), (c2.center - ((((uu + vv)) * c2.r) * s))));
      ret.push_back(Line((c1.center + (((uu - vv)) * c1.r)), (c2.center - ((((uu - vv)) * c2.r) * s))));
    }
  }
  return ret;
}

func MinimumBoundingCircle(v: dynamic) -> dynamic
{
  var n: dynamic = v.size();
  var mt: dynamic = cpp_construct(time(0));
  shuffle(v.begin(), v.end(), mt);
  var ret: dynamic = cpp_construct(0, 0);
  var make_circle2: dynamic = __cpp_lambda_1;
  var make_circle3: dynamic = __cpp_lambda_2;
  var isIn: dynamic = __cpp_lambda_3;
  ret = make_circle2(v[0], v[1]);
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      if ((!isIn(v[i])))
      {
        ret = make_circle2(v[0], v[i]);
        {
          var j: dynamic = 1;
          while ((j < i))
          {
            if ((!isIn(v[j])))
            {
              ret = make_circle2(v[i], v[j]);
              {
                var k: dynamic = 0;
                while ((k < j))
                {
                  if ((!isIn(v[k])))
                  {
                    ret = make_circle3(v[i], v[j], v[k]);
                  }
                  k += 1;
                }
              }
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  return ret;
}

func pow_mod(x: dynamic, n: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return 1;
  }
  var ret: dynamic = pow_mod((((x * x)) % mod), (n / 2));
  if ((n & 1))
  {
    ret = (((ret * x)) % mod);
  }
  return ret;
}

func chromatic_number(g: dynamic) -> dynamic
{
  var n: dynamic = cpp_cast(g.size());
  var es: dynamic = cpp_construct(n, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          es[i] |= ((g[i][j] << j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var I: dynamic = cpp_construct((1 << n));
  I[0] = 1;
  {
    var S: dynamic = 1;
    while ((S < ((1 << n))))
    {
      var v: dynamic = builtin_ctz(S);
      I[S] = (I[(S ^ ((1 << v)))] + I[(((S ^ ((1 << v)))) & ((~es[v])))]);
      S += 1;
    }
  }
  var lw: dynamic = 0;
  var hi: dynamic = n;
  while (((hi - lw) > 1))
  {
    var mid: dynamic = (((lw + hi)) / 2);
    var g: dynamic = 0;
    {
      var S: dynamic = 0;
      while ((S < ((1 << n))))
      {
        if ((((n - builtin_popcount(S))) & 1))
        {
          g -= pow_mod(I[S], mid);
        } else
        {
          g += pow_mod(I[S], mid);
        }
        g = ((((g % mod) + mod)) % mod);
        S += 1;
      }
    }
    if ((g != 0))
    {
      hi = mid;
    } else
    {
      lw = mid;
    }
  }
  return hi;
}

var n: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  write(chromatic_number(g), "\n");
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  while (cpp_comma((cin >> n), n))
  {
    solve();
  }
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return Circle((((a + b)) * 0.5), (dis(a, b) / 2));
}

func __cpp_lambda_2(A: dynamic, B: dynamic, C: dynamic) -> dynamic
{
  var cent: dynamic = circumcenter(A, B, C);
  return Circle(cent, dis(cent, A));
}

func __cpp_lambda_3(a: dynamic) -> dynamic
{
  return (dis(ret.center, a) < (ret.r + EPS));
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var m: dynamic = cpp_uninitialized();
    read(m);
    var pre: dynamic = cpp_uninitialized();
    read(pre);
    rep(j, (m - 1));
    {
      var p: dynamic = cpp_uninitialized();
      read(p);
      ls[i].push_back(Segment(p, pre));
      pre = p;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    {
      var j: dynamic = (i + 1);
      while ((j < n))
      {
        var f: dynamic = false;
        for (var a: dynamic in ls[i])
        {
          for (var b: dynamic in ls[j])
          {
            if (intersect(a, b))
            {
              f = true;
            }
          }
        }
        g[i][j] = f;
        g[j][i] = f;
        j += 1;
      }
    }
  }
