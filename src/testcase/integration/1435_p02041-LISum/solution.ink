// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var inf: dynamic = cpp_expression("#inclu");

var pa: dynamic = cpp_expression("#include");

var ll: dynamic = dynamic;

var pal: dynamic = cpp_expression("#include <bits/");

var ppap: dynamic = cpp_expression("#include");

var PI: dynamic = cpp_expression("#include <bits/std");

var paa: dynamic = cpp_expression("#include <");

var mp: dynamic = cpp_expression("#incl");

var pb: dynamic = cpp_expression("#incl");

var EPS: dynamic = cpp_expression("#in");

class pa3
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  func pa3(x: dynamic = 0, y: dynamic = 0, z: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
      self->z = cpp_construct(z);
    }
  func operator_less(p: dynamic) -> dynamic
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
  func operator_greater(p: dynamic) -> dynamic
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
  func operator_equal(p: dynamic) -> dynamic
  {
      return (((x == p.x) && (y == p.y)) && (z == p.z));
    }
  func operator_not_equal(p: dynamic) -> dynamic
  {
      return (!((((x == p.x) && (y == p.y)) && (z == p.z))));
    }
}

class pa4
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  func pa4(x: dynamic = 0, y: dynamic = 0, z: dynamic = 0, w: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
      self->z = cpp_construct(z);
      self->w = cpp_construct(w);
    }
  func operator_less(p: dynamic) -> dynamic
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
  func operator_greater(p: dynamic) -> dynamic
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
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((((x == p.x) && (y == p.y)) && (z == p.z)) && (w == p.w));
    }
}

class pa2
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func pa2(x: dynamic = 0, y: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func operator_add(p: dynamic) -> dynamic
  {
      return pa2((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic) -> dynamic
  {
      return pa2((x - p.x), (y - p.y));
    }
  func operator_less(p: dynamic) -> dynamic
  {
      return  ((y != p.y)) ? (y < p.y) : (x < p.x);
    }
  func operator_greater(p: dynamic) -> dynamic
  {
      return  ((x != p.x)) ? (x < p.x) : (y < p.y);
    }
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((abs((x - p.x)) == 0) && (abs((y - p.y)) == 0));
    }
  func operator_not_equal(p: dynamic) -> dynamic
  {
      return (!(((abs((x - p.x)) == 0) && (abs((y - p.y)) == 0))));
    }
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = 0, y: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func operator_add(p: dynamic) -> dynamic
  {
      return Point((x + p.x), (y + p.y));
    }
  func operator_subtract(p: dynamic) -> dynamic
  {
      return Point((x - p.x), (y - p.y));
    }
  func operator_multiply(a: dynamic) -> dynamic
  {
      return Point((x * a), (y * a));
    }
  func operator_divide(a: dynamic) -> dynamic
  {
      return Point((x / a), (y / a));
    }
  func absv() -> dynamic
  {
      return sqrt(norm());
    }
  func norm() -> dynamic
  {
      return ((x * x) + (y * y));
    }
  func operator_less(p: dynamic) -> dynamic
  {
      return  ((x != p.x)) ? (x < p.x) : (y < p.y);
    }
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((fabs((x - p.x)) < EPS) && (fabs((y - p.y)) < EPS));
    }
}

var pl: dynamic = cpp_expression("#include");

class Segment
{
  var p1: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func parareru(a: dynamic, b: dynamic, c: dynamic, d: dynamic) -> dynamic
{
  return (abs(cross((a - b), (d - c))) < EPS);
}

func distance_ls_p(a: dynamic, b: dynamic, c: dynamic) -> dynamic
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

func is_intersected_ls(a: dynamic, b: dynamic) -> dynamic
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

func segment_dis(a: dynamic, b: dynamic) -> dynamic
{
  if (is_intersected_ls(a, b))
  {
    return 0;
  }
  var r: dynamic = distance_ls_p(a.p1, a.p2, b.p1);
  r = min(r, distance_ls_p(a.p1, a.p2, b.p2));
  r = min(r, distance_ls_p(b.p1, b.p2, a.p2));
  r = min(r, distance_ls_p(b.p1, b.p2, a.p1));
  return r;
}

func intersection_ls(a: dynamic, b: dynamic) -> dynamic
{
  var ba: dynamic = (b.p2 - b.p1);
  var d1: dynamic = abs(cross(ba, (a.p1 - b.p1)));
  var d2: dynamic = abs(cross(ba, (a.p2 - b.p1)));
  var t: dynamic = (d1 / ((d1 + d2)));
  return (a.p1 + (((a.p2 - a.p1)) * t));
}

func itos(i: dynamic) -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  (s << i);
  return s.str();
}

func gcd(v: dynamic, b: dynamic) -> dynamic
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

func distans(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic) -> dynamic
{
  var rr: dynamic = ((((x1 - x2)) * ((x1 - x2))) + (((y1 - y2)) * ((y1 - y2))));
  return sqrt(rr);
}

var mod: dynamic = cpp_uninitialized();

func extgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var d: dynamic = extgcd(b, (a % b), y, x);
  y -= ((a / b) * x);
  return d;
}

func operator_add(l: dynamic, r: dynamic) -> dynamic
{
  return [(l.first + r.first), (l.second + r.second)];
}

func operator_subtract(l: dynamic, r: dynamic) -> dynamic
{
  return [(l.first - r.first), (l.second - r.second)];
}

var pr: dynamic = cpp_array(200010);

var inv: dynamic = cpp_array(200010);

func beki(wa: dynamic, rr: dynamic, warukazu: dynamic) -> dynamic
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
  var zx: dynamic = beki(wa, (rr / 2), warukazu);
  return (((zx * zx)) % warukazu);
}

func bekid(w: dynamic, r: dynamic) -> dynamic
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
  var f: dynamic = bekid(w, (r / 2));
  return (f * f);
}

func comb(nn: dynamic, rr: dynamic) -> dynamic
{
  var r: dynamic = (pr[nn] * inv[rr]);
  r %= mod;
  r *= inv[(nn - rr)];
  r %= mod;
  return r;
}

func gya(ert: dynamic) -> dynamic
{
  pr[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= ert))
    {
      pr[i] = (((pr[(i - 1)] * i)) % mod);
      i += 1;
    }
  }
  inv[ert] = beki(pr[ert], (mod - 2), mod);
  {
    var i: dynamic = (ert - 1);
    while ((i >= 0))
    {
      inv[i] = ((inv[(i + 1)] * ((i + 1))) % mod);
      i -= 1;
    }
  }
}

class Segmax
{
  var cor: dynamic = cpp_uninitialized();
  var vec: dynamic = cpp_uninitialized();
  func shoki1() -> dynamic
  {
      vec.resize(((2 * cor) + 3), mp(-1, 0));
    }
  func shoki2() -> dynamic
  {
      {
        var i: dynamic = (cor - 1);
        while ((i > 0))
        {
          vec[i] = max(vec[(2 * i)], vec[((2 * i) + 1)]);
          i -= 1;
        }
      }
    }
  func updchan(x: dynamic, w: dynamic) -> dynamic
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
  func segmax(a: dynamic, b: dynamic, k: dynamic = 1, l: dynamic = 0, r: dynamic = -10) -> dynamic
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
      var v1: dynamic = segmax(a, b, (k * 2), l, (((l + r)) / 2));
      var v2: dynamic = segmax(a, b, ((k * 2) + 1), (((l + r)) / 2), r);
      return max(v1, v2);
    }
}

var ss: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  read(n);
  ss.shoki1();
  ss.shoki2();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var y: dynamic = cpp_uninitialized();
      read(y);
      var z: dynamic = ss.segmax(0, y);
      var r: dynamic = cpp_uninitialized();
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
  var ans: dynamic = ss.segmax(0, ss.cor);
  write(ans.second, "\n");
  return 0;
}
