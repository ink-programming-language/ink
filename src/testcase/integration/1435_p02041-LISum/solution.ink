// Translated from solution.cpp.

var int_cpp = dynamic;

var inf = cpp_expression("#inclu");

var pa = cpp_expression("#include");

var ll = dynamic;

var pal = cpp_expression("#include <bits/");

var ppap = cpp_expression("#include");

var PI = cpp_expression("#include <bits/std");

var paa = cpp_expression("#include <");

var mp = cpp_expression("#incl");

var pb = cpp_expression("#incl");

var EPS = cpp_expression("#in");

class pa3
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  func pa3(x: dynamic = 0, y: dynamic = 0, z: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
      this->z = cpp_construct(z);
    }
  func operator_less(p: dynamic)
  {
      if ((x != p.x))
      {
        return (x < p.x);
      }
      if ((y != p.y))
      {
        return (y < p.y);
      }
      return (z < p.z);
    }
  func operator_greater(p: dynamic)
  {
      if ((x != p.x))
      {
        return (x > p.x);
      }
      if ((y != p.y))
      {
        return (y > p.y);
      }
      return (z > p.z);
    }
  func operator_equal(p: dynamic)
  {
      return (((x == p.x) && (y == p.y)) && (z == p.z));
    }
  func operator_not_equal(p: dynamic)
  {
      return (!((((x == p.x) && (y == p.y)) && (z == p.z))));
    }
}

class pa4
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  var w: dynamic;
  func pa4(x: dynamic = 0, y: dynamic = 0, z: dynamic = 0, w: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
      this->z = cpp_construct(z);
      this->w = cpp_construct(w);
    }
  func operator_less(p: dynamic)
  {
      if ((x != p.x))
      {
        return (x < p.x);
      }
      if ((y != p.y))
      {
        return (y < p.y);
      }
      if ((z != p.z))
      {
        return (z < p.z);
      }
      return (w < p.w);
    }
  func operator_greater(p: dynamic)
  {
      if ((x != p.x))
      {
        return (x > p.x);
      }
      if ((y != p.y))
      {
        return (y > p.y);
      }
      if ((z != p.z))
      {
        return (z > p.z);
      }
      return (w > p.w);
    }
  func operator_equal(p: dynamic)
  {
      return ((((x == p.x) && (y == p.y)) && (z == p.z)) && (w == p.w));
    }
}

class pa2
{
  var x: dynamic;
  var y: dynamic;
  func pa2(x: dynamic = 0, y: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_add(p: dynamic)
  {
      return pa2((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic)
  {
      return pa2((x - p.x), (y - p.y));
    }
  func operator_less(p: dynamic)
  {
      return if ((y != p.y)) (y < p.y) else (x < p.x);
    }
  func operator_greater(p: dynamic)
  {
      return if ((x != p.x)) (x < p.x) else (y < p.y);
    }
  func operator_equal(p: dynamic)
  {
      return ((abs((x - p.x)) == 0) && (abs((y - p.y)) == 0));
    }
  func operator_not_equal(p: dynamic)
  {
      return (!(((abs((x - p.x)) == 0) && (abs((y - p.y)) == 0))));
    }
}

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point(x: dynamic = 0, y: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_add(p: dynamic)
  {
      return Point((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic)
  {
      return Point((x - p.x), (y - p.y));
    }
  func operator_multiply(a: dynamic)
  {
      return Point((x * a), (y * a));
    }
  func operator_divide(a: dynamic)
  {
      return Point((x / a), (y / a));
    }
  func absv()
  {
      return sqrt(norm());
    }
  func norm()
  {
      return ((x * x) + (y * y));
    }
  func operator_less(p: dynamic)
  {
      return if ((x != p.x)) (x < p.x) else (y < p.y);
    }
  func operator_equal(p: dynamic)
  {
      return ((fabs((x - p.x)) < EPS) && (fabs((y - p.y)) < EPS));
    }
}

var pl = cpp_expression("#include");

class Segment
{
  var p1: dynamic;
  var p2: dynamic;
}

func dot(a: dynamic, b: dynamic)
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func parareru(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  return (abs(cross((a - b), (d - c))) < EPS);
}

func distance_ls_p(a: dynamic, b: dynamic, c: dynamic)
{
  if ((dot((b - a), (c - a)) < EPS))
  {
    return ((c - a)).absv();
  }
  if ((dot((a - b), (c - b)) < EPS))
  {
    return ((c - b)).absv();
  }
  return (abs(cross((b - a), (c - a))) / ((b - a)).absv());
}

func is_intersected_ls(a: dynamic, b: dynamic)
{
  if (((((a.p1 == b.p1) || (a.p2 == b.p1)) || (a.p1 == b.p2)) || (a.p2 == b.p2)))
  {
    return false;
  }
  if ((parareru((a.p2), (a.p1), (a.p1), (b.p2)) && parareru((a.p2), (a.p1), (a.p1), (b.p1))))
  {
    if ((dot((a.p1 - b.p1), (a.p1 - b.p2)) < EPS))
    {
      return true;
    }
    if ((dot((a.p2 - b.p1), (a.p2 - b.p2)) < EPS))
    {
      return true;
    }
    if ((dot((a.p1 - b.p1), (a.p2 - b.p1)) < EPS))
    {
      return true;
    }
    if ((dot((a.p1 - b.p2), (a.p2 - b.p2)) < EPS))
    {
      return true;
    }
    return false;
  } else
  {
    return ((((cross((a.p2 - a.p1), (b.p1 - a.p1)) * cross((a.p2 - a.p1), (b.p2 - a.p1))) < EPS)) && (((cross((b.p2 - b.p1), (a.p1 - b.p1)) * cross((b.p2 - b.p1), (a.p2 - b.p1))) < EPS)));
  }
}

func segment_dis(a: dynamic, b: dynamic)
{
  if (is_intersected_ls(a, b))
  {
    return 0;
  }
  var r = distance_ls_p(a.p1, a.p2, b.p1);
  r = min(r, distance_ls_p(a.p1, a.p2, b.p2));
  r = min(r, distance_ls_p(b.p1, b.p2, a.p2));
  r = min(r, distance_ls_p(b.p1, b.p2, a.p1));
  return r;
}

func intersection_ls(a: dynamic, b: dynamic)
{
  var ba = (b.p2 - b.p1);
  var d1 = abs(cross(ba, (a.p1 - b.p1)));
  var d2 = abs(cross(ba, (a.p2 - b.p1)));
  var t = (d1 / ((d1 + d2)));
  return (a.p1 + (((a.p2 - a.p1)) * t));
}

func itos(i: dynamic)
{
  var s: dynamic;
  (s << i);
  return s.str();
}

func gcd(v: dynamic, b: dynamic)
{
  if ((v > b))
  {
    return gcd(b, v);
  }
  if ((v == b))
  {
    return b;
  }
  if (((b % v) == 0))
  {
    return v;
  }
  return gcd(v, (b % v));
}

func distans(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
{
  var rr = ((((x1 - x2)) * ((x1 - x2))) + (((y1 - y2)) * ((y1 - y2))));
  return sqrt(rr);
}

var mod: dynamic;

func extgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var d = extgcd(b, (a % b), y, x);
  y -= ((a / b) * x);
  return d;
}

func operator_add(l: dynamic, r: dynamic)
{
  return [(l.first + r.first), (l.second + r.second)];
}

func operator_subtract(l: dynamic, r: dynamic)
{
  return [(l.first - r.first), (l.second - r.second)];
}

var pr = cpp_array(200010);

var inv = cpp_array(200010);

func beki(wa: dynamic, rr: dynamic, warukazu: dynamic)
{
  if ((rr == 0))
  {
    return (1 % warukazu);
  }
  if ((rr == 1))
  {
    return (wa % warukazu);
  }
  wa %= warukazu;
  if (((rr % 2) == 1))
  {
    return (((cpp_cast(beki(wa, (rr - 1), warukazu)) * cpp_cast(wa))) % warukazu);
  }
  var zx = beki(wa, (rr / 2), warukazu);
  return (((zx * zx)) % warukazu);
}

func bekid(w: dynamic, r: dynamic)
{
  if ((r == 0))
  {
    return 1.0;
  }
  if ((r == 1))
  {
    return w;
  }
  if ((r % 2))
  {
    return (bekid(w, (r - 1)) * w);
  }
  var f = bekid(w, (r / 2));
  return (f * f);
}

func comb(nn: dynamic, rr: dynamic)
{
  var r = (pr[nn] * inv[rr]);
  r %= mod;
  r *= inv[(nn - rr)];
  r %= mod;
  return r;
}

func gya(ert: dynamic)
{
  pr[0] = 1;
  {
    var i = 1;
    while ((i <= ert))
    {
      pr[i] = (((pr[(i - 1)] * i)) % mod);
      i += 1;
    }
  }
  inv[ert] = beki(pr[ert], (mod - 2), mod);
  {
    var i = (ert - 1);
    while ((i >= 0))
    {
      inv[i] = ((inv[(i + 1)] * ((i + 1))) % mod);
      i -= 1;
    }
  }
}

class Segmax
{
  var cor: dynamic;
  var vec: dynamic;
  func shoki1()
  {
      vec.resize(((2 * cor) + 3), mp(-1, 0));
    }
  func shoki2()
  {
      {
        var i = (cor - 1);
        while ((i > 0))
        {
          vec[i] = max(vec[(2 * i)], vec[((2 * i) + 1)]);
          i -= 1;
        }
      }
    }
  func updchan(x: dynamic, w: dynamic)
  {
      x += cor;
      vec[x] = w;
      while (1)
      {
        x /= 2;
        if ((x == 0))
        {
          break;
        }
        vec[x] = max(vec[((2 * x) + 1)], vec[(2 * x)]);
      }
    }
  func segmax(a: dynamic, b: dynamic, k: dynamic = 1, l: dynamic = 0, r: dynamic = -10)
  {
      if ((r < 0))
      {
        r = cor;
      }
      if (((a <= l) && (r <= b)))
      {
        return vec[k];
      }
      if (((r <= a) || (b <= l)))
      {
        return mp(-1, -1);
      }
      var v1 = segmax(a, b, (k * 2), l, (((l + r)) / 2));
      var v2 = segmax(a, b, ((k * 2) + 1), (((l + r)) / 2), r);
      return max(v1, v2);
    }
}

var ss: dynamic;

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic;
  read(n);
  ss.shoki1();
  ss.shoki2();
  {
    var i = 0;
    while ((i < n))
    {
      var y: dynamic;
      read(y);
      var z = ss.segmax(0, y);
      var r: dynamic;
      if ((z.first < 0))
      {
        r = [1, y];
      } else
      {
        r = z;
        r.first += 1;
        r.second += y;
      }
      if ((ss.vec[(ss.cor + y)] < r))
      {
        ss.updchan(y, r);
      }
      i += 1;
    }
  }
  var ans = ss.segmax(0, ss.cor);
  write(ans.second, "\n");
  return 0;
}
