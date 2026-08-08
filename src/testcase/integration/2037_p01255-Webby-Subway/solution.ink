// Translated from solution.cpp.

func ALL(x: dynamic)
{
  return cpp_expression("#include<bits/std");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func debug(v: dynamic)
{
  cpp_macro("cout<<#v<<\":\";for(auto x:v){cout<<x<<' ';}cout<<endl;");
}

var INF = cpp_expression("#include<b");

var mod = cpp_expression("#include<b");

var LINF = 1001002003004005006;

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
    return true;
  }
  return false;
}

var EPS = 1e-10;

var pi = acosl(-1);

func operator_shift_right(is: dynamic, p: dynamic)
{
  var a: dynamic;
  var b: dynamic;
  ((is >> a) >> b);
  p = Point(a, b);
  return is;
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << fixed) << setprecision(12)) << p.real()) << cpp_char(" ")) << p.imag());
}

func eq(a: dynamic, b: dynamic)
{
  return (fabs((a - b)) < EPS);
}

func operator_multiply(p: dynamic, d: dynamic)
{
  return Point((real(p) * d), (imag(p) * d));
}

class Line
{
  var p1: dynamic;
  var p2: dynamic;
  func Line()
  {
    }
  func Line(p1: dynamic, p2: dynamic)
  {
      this->p1 = cpp_construct(p1);
      this->p2 = cpp_construct(p2);
    }
  func Line(A: dynamic, B: dynamic, C: dynamic)
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
  func Segment()
  {
    }
  func Segment(p1: dynamic, p2: dynamic)
  {
      this->Line = cpp_construct(p1, p2);
    }
}

class Circle
{
  var center: dynamic;
  var r: dynamic;
  func Circle()
  {
    }
  func Circle(center: dynamic, r: dynamic)
  {
      this->center = cpp_construct(center);
      this->r = cpp_construct(r);
    }
}

func rotate(theta: dynamic, p: dynamic)
{
  return Point(((cos(theta) * p.real()) - (sin(theta) * p.imag())), ((sin(theta) * p.real()) + (cos(theta) * p.imag())));
}

func radian_to_degree(r: dynamic)
{
  return ((r * 180.0) / pi);
}

func degree_to_radian(d: dynamic)
{
  return ((d * pi) / 180.0);
}

func area_triangle(a: dynamic, b: dynamic, c: dynamic)
{
  var x = (b - a);
  var y = (c - a);
  return (fabs(((x.real() * y.imag()) - (x.imag() * y.real()))) / 2);
}

func cross(a: dynamic, b: dynamic)
{
  return ((real(a) * imag(b)) - (imag(a) * real(b)));
}

func dot(a: dynamic, b: dynamic)
{
  return ((real(a) * real(b)) + (imag(a) * imag(b)));
}

func parallel(a: dynamic, b: dynamic)
{
  return eq(cross((a.p1 - a.p2), (b.p1 - b.p2)), 0.0);
}

func orthogonal(a: dynamic, b: dynamic)
{
  return eq(dot((a.p1 - a.p2), (b.p1 - b.p2)), 0.0);
}

func projection(l: dynamic, p: dynamic)
{
  var k = (dot((l.p1 - l.p2), (p - l.p1)) / norm((l.p1 - l.p2)));
  return (l.p1 + (((l.p1 - l.p2)) * k));
}

func projection(l: dynamic, p: dynamic)
{
  var k = (dot((l.p1 - l.p2), (p - l.p1)) / norm((l.p1 - l.p2)));
  return (l.p1 + (((l.p1 - l.p2)) * k));
}

func reflection(l: dynamic, p: dynamic)
{
  var h = projection(l, p);
  return (((p + ((h - p))) + ((h - p))));
}

func reflection(l: dynamic, p: dynamic)
{
  var h = projection(l, p);
  return (((p + ((h - p))) + ((h - p))));
}

func dis(a: dynamic, b: dynamic)
{
  return abs((a - b));
}

func dis(l: dynamic, p: dynamic)
{
  return abs((p - projection(l, p)));
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
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

func circumcenter(A: dynamic, B: dynamic, C: dynamic)
{
  var S = area_triangle(A, B, C);
  var a = dis(B, C);
  var b = dis(A, C);
  var c = dis(A, B);
  return (((A * ((((a * a) * ((((b * b) + (c * c)) - (a * a)))) / (((16 * S) * S))))) + (B * ((((b * b) * ((((c * c) + (a * a)) - (b * b)))) / (((16 * S) * S)))))) + (C * ((((c * c) * ((((a * a) + (b * b)) - (c * c)))) / (((16 * S) * S))))));
}

func intersect(l: dynamic, p: dynamic)
{
  return (abs(ccw(l.p1, l.p2, p)) != 1);
}

func intersect(l1: dynamic, l2: dynamic)
{
  return cpp_binary((abs(cross((l1.p2 - l1.p1), (l2.p2 - l2.p1))) > EPS), "or", (abs(cross((l1.p2 - l1.p1), (l2.p2 - l1.p1))) < EPS));
}

func intersect(s: dynamic, p: dynamic)
{
  return (ccw(s.p1, s.p2, p) == 0);
}

func intersect(l: dynamic, s: dynamic)
{
  return ((cross((l.p2 - l.p1), (s.p1 - l.p1)) * cross((l.p2 - l.p1), (s.p2 - l.p1))) < EPS);
}

func intersect(c: dynamic, l: dynamic)
{
  return (dis(l, c.center) <= (c.r + EPS));
}

func intersect(c: dynamic, p: dynamic)
{
  return (abs((abs((p - c.center)) - c.r)) < EPS);
}

func intersect(s: dynamic, t: dynamic)
{
  return cpp_binary(((ccw(s.p1, s.p2, t.p1) * ccw(s.p1, s.p2, t.p2)) <= 0), "and", ((ccw(t.p1, t.p2, s.p1) * ccw(t.p1, t.p2, s.p2)) <= 0));
}

func intersect(c: dynamic, l: dynamic)
{
  var h = projection(l, c.center);
  if (((norm((h - c.center)) - (c.r * c.r)) > EPS))
  {
    return 0;
  }
  var d1 = abs((c.center - l.p1));
  var d2 = abs((c.center - l.p2));
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

func intersect(c1: dynamic, c2: dynamic)
{
  if ((c1.r < c2.r))
  {
    swap(c1, c2);
  }
  var d = abs((c1.center - c2.center));
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

func crosspoint(l: dynamic, m: dynamic)
{
  var A = cross((m.p2 - m.p1), (m.p1 - l.p1));
  var B = cross((m.p2 - m.p1), (l.p2 - l.p1));
  if (cpp_binary(eq(A, 0), "and", eq(B, 0)))
  {
    return l.p1;
  }
  if (eq(B, 0))
  {
    throw "NAI";
  }
  return (l.p1 + ((A / B) * ((l.p2 - l.p1))));
}

func crosspoint(l: dynamic, m: dynamic)
{
  return crosspoint(Line(l), Line(m));
}

func crosspoint(c: dynamic, l: dynamic)
{
  var ret: dynamic;
  var h = projection(l, c.center);
  var d = sqrt(((c.r * c.r) - norm((h - c.center))));
  var e = (((l.p2 - l.p1)) * ((1 / abs((l.p2 - l.p1)))));
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

func crosspoint(c: dynamic, s: dynamic)
{
  var l = Line(s.p1, s.p2);
  var ko = intersect(c, s);
  if ((ko == 2))
  {
    return crosspoint(c, l);
  }
  var ret: dynamic;
  if ((ko == 0))
  {
    return ret;
  }
  ret = crosspoint(c, l);
  if ((ret.size() == 1))
  {
    return ret;
  }
  var rret: dynamic;
  if ((dot((s.p1 - ret[0]), (s.p2 - ret[0])) < 0))
  {
    rret.push_back(ret[0]);
  } else
  {
    rret.push_back(ret[1]);
  }
  return rret;
}

func crosspoint(c1: dynamic, c2: dynamic)
{
  var ret: dynamic;
  var isec = intersect(c1, c2);
  if (cpp_binary((isec == 0), "or", (isec == 4)))
  {
    return ret;
  }
  var d = abs((c1.center - c2.center));
  var a = acos((((((c1.r * c1.r) + (d * d)) - (c2.r * c2.r))) / (((2 * c1.r) * d))));
  var t = atan2((c2.center.imag() - c1.center.imag()), (c2.center.real() - c1.center.real()));
  ret.push_back((c1.center + Point((cos((t + a)) * c1.r), (sin((t + a)) * c1.r))));
  ret.push_back((c1.center + Point((cos((t - a)) * c1.r), (sin((t - a)) * c1.r))));
  return ret;
}

func tangent(c: dynamic, p: dynamic)
{
  return crosspoint(c, Circle(p, sqrt((norm((c.center - p)) - (c.r * c.r)))));
}

func tangent(c1: dynamic, c2: dynamic)
{
  var ret: dynamic;
  if ((c1.r < c2.r))
  {
    swap(c1, c2);
  }
  var g = norm((c1.center - c2.center));
  if (eq(g, 0))
  {
    return ret;
  }
  var u = (((c2.center - c1.center)) / sqrt(g));
  var v = rotate((pi * 0.5), u);
  for (var s in [-1, 1])
  {
    var h = (((c1.r + (s * c2.r))) / sqrt(g));
    if (eq((1 - (h * h)), 0))
    {
      ret.push_back(Line((c1.center + (u * c1.r)), (c1.center + (((u + v)) * c1.r))));
    } else if (((1 - (h * h)) > 0))
    {
      var uu = (u * h);
      var vv = (v * sqrt((1 - (h * h))));
      ret.push_back(Line((c1.center + (((uu + vv)) * c1.r)), (c2.center - ((((uu + vv)) * c2.r) * s))));
      ret.push_back(Line((c1.center + (((uu - vv)) * c1.r)), (c2.center - ((((uu - vv)) * c2.r) * s))));
    }
  }
  return ret;
}

func MinimumBoundingCircle(v: dynamic)
{
  var n = v.size();
  var mt = cpp_construct(time(0));
  shuffle(v.begin(), v.end(), mt);
  var ret = cpp_construct(0, 0);
  var make_circle2 = __cpp_lambda_1;
  var make_circle3 = __cpp_lambda_2;
  var isIn = __cpp_lambda_3;
  ret = make_circle2(v[0], v[1]);
  {
    var i = 2;
    while ((i < n))
    {
      if ((!isIn(v[i])))
      {
        ret = make_circle2(v[0], v[i]);
        {
          var j = 1;
          while ((j < i))
          {
            if ((!isIn(v[j])))
            {
              ret = make_circle2(v[i], v[j]);
              {
                var k = 0;
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

func pow_mod(x: dynamic, n: dynamic)
{
  if ((n == 0))
  {
    return 1;
  }
  var ret = pow_mod((((x * x)) % mod), (n / 2));
  if ((n & 1))
  {
    ret = (((ret * x)) % mod);
  }
  return ret;
}

func chromatic_number(g: dynamic)
{
  var n = cpp_cast(g.size());
  var es = cpp_construct(n, 0);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          es[i] |= ((g[i][j] << j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var I = cpp_construct((1 << n));
  I[0] = 1;
  {
    var S = 1;
    while ((S < ((1 << n))))
    {
      var v = builtin_ctz(S);
      I[S] = (I[(S ^ ((1 << v)))] + I[(((S ^ ((1 << v)))) & ((~es[v])))]);
      S += 1;
    }
  }
  var lw = 0;
  var hi = n;
  while (((hi - lw) > 1))
  {
    var mid = (((lw + hi)) / 2);
    var g = 0;
    {
      var S = 0;
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

var n: dynamic;

func solve()
{
  write(chromatic_number(g), "\n");
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  while (cpp_comma((cin >> n), n))
  {
    solve();
  }
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return Circle((((a + b)) * 0.5), (dis(a, b) / 2));
}

func __cpp_lambda_2(A: dynamic, B: dynamic, C: dynamic)
{
  var cent = circumcenter(A, B, C);
  return Circle(cent, dis(cent, A));
}

func __cpp_lambda_3(a: dynamic)
{
  return (dis(ret.center, a) < (ret.r + EPS));
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var m: dynamic;
    read(m);
    var pre: dynamic;
    read(pre);
    rep(j, (m - 1));
    {
      var p: dynamic;
      read(p);
      ls[i].push_back(Segment(p, pre));
      pre = p;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    {
      var j = (i + 1);
      while ((j < n))
      {
        var f = false;
        for (var a in ls[i])
        {
          for (var b in ls[j])
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
