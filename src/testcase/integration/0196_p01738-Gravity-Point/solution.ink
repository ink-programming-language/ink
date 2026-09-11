// Translated from solution.cpp.

var EPS: dynamic = 1e-9;

class L
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  func L(A: dynamic, B: dynamic, C: dynamic) -> dynamic
  {
      a = A;
      b = B;
      c = C;
    }
  func L() -> dynamic
  {
    }
}

class P
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func P(X: dynamic, Y: dynamic) -> dynamic
  {
      x = X;
      y = Y;
    }
  func P() -> dynamic
  {
    }
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.x != b.x))
  {
    return (a.x < b.x);
  } else
  {
    return (a.y < b.y);
  }
}

func inter(s: dynamic, t: dynamic) -> dynamic
{
  var det: dynamic = ((s.a * t.b) - (s.b * t.a));
  if ((abs(det) < EPS))
  {
    return P(1000000009, 1000000009);
  }
  return P(((((t.b * s.c) - (s.b * t.c))) / det), ((((t.c * s.a) - (s.c * t.a))) / det));
}

func eq(a: dynamic, b: dynamic) -> dynamic
{
  return ((abs((a.x - b.x)) < EPS) && (abs((a.y - b.y)) < EPS));
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func norm(a: dynamic) -> dynamic
{
  return ((a.x * a.x) + (a.y * a.y));
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b.x -= a.x;
  b.y -= a.y;
  c.x -= a.x;
  c.y -= a.y;
  if ((cross(b, c) > 0))
  {
    return 1;
  }
  if ((cross(b, c) < 0))
  {
    return -1;
  }
  if ((dot(b, c) < 0))
  {
    return 2;
  }
  if ((norm(b) < norm(c)))
  {
    return -2;
  }
  return 0;
}

func convex_hull(ps: dynamic) -> dynamic
{
  var n: dynamic = ps.size();
  var k: dynamic = 0;
  sort(ps.begin(), ps.end());
  var ch: dynamic = cpp_construct((2 * n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      while (((k >= 2) && (ccw(ch[(k - 2)], ch[(k - 1)], ps[i]) <= 0)))
      {
        k -= 1;
      }
      ch[cpp_update(k, "++")] = ps[cpp_update(i, "++")];
    }
  }
  {
    var i: dynamic = (n - 2);
    var t: dynamic = (k + 1);
    while ((i >= 0))
    {
      while (((k >= t) && (ccw(ch[(k - 2)], ch[(k - 1)], ps[i]) <= 0)))
      {
        k -= 1;
      }
      ch[cpp_update(k, "++")] = ps[cpp_update(i, "--")];
    }
  }
  ch.resize((k - 1));
  return ch;
}

var str: dynamic = cpp_array(51, 51);

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  scanf("%d%d", (&a), (&b));
  var mA1: dynamic = cpp_uninitialized();
  var mA2: dynamic = cpp_uninitialized();
  var mB1: dynamic = cpp_uninitialized();
  var mB2: dynamic = cpp_uninitialized();
  var mX: dynamic = cpp_uninitialized();
  scanf("%lf%lf%lf%lf%lf", (&mA1), (&mA2), (&mB1), (&mB2), (&mX));
  {
    var i: dynamic = 0;
    while ((i < a))
    {
      scanf("%s", str[i]);
      i += 1;
    }
  }
  var tx: dynamic = 0;
  var ux: dynamic = 0;
  var ty: dynamic = 0;
  var uy: dynamic = 0;
  var vx: dynamic = 0;
  var vy: dynamic = 0;
  var wa: dynamic = 0;
  var wb: dynamic = 0;
  var wc: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < a))
    {
      {
        var j: dynamic = 0;
        while ((j < b))
        {
          if ((str[i][j] == cpp_char("A")))
          {
            wa += 1;
            tx += (0.5 + i);
            ty += (0.5 + j);
          } else if ((str[i][j] == cpp_char("B")))
          {
            wb += 1;
            ux += (0.5 + i);
            uy += (0.5 + j);
          } else if ((str[i][j] == cpp_char("X")))
          {
            wc += 1;
            vx += (0.5 + i);
            vy += (0.5 + j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  wc *= mX;
  vx *= mX;
  vy *= mX;
  var ret: dynamic = 0;
  var div: dynamic = (((mA2 - mA1)) * ((mB2 - mB1)));
  {
    var i: dynamic = 0;
    while ((i < a))
    {
      {
        var j: dynamic = 0;
        while ((j < b))
        {
          if ((str[i][j] == cpp_char(".")))
          {
            j += 1;
            continue;
          }
          var xl: dynamic = i;
          var xh: dynamic = (i + 1);
          var yl: dynamic = j;
          var yh: dynamic = (j + 1);
          var lines: dynamic = cpp_uninitialized();
          lines.push_back(L(1, 0, mA1));
          lines.push_back(L(1, 0, mA2));
          lines.push_back(L(0, 1, mB1));
          lines.push_back(L(0, 1, mB2));
          lines.push_back(L((tx - (wa * xl)), (ux - (wb * xl)), ((-vx) + (wc * xl))));
          lines.push_back(L((tx - (wa * xh)), (ux - (wb * xh)), ((-vx) + (wc * xh))));
          lines.push_back(L((ty - (wa * yl)), (uy - (wb * yl)), ((-vy) + (wc * yl))));
          lines.push_back(L((ty - (wa * yh)), (uy - (wb * yh)), ((-vy) + (wc * yh))));
          var cons: dynamic = cpp_uninitialized();
          {
            var k: dynamic = 0;
            while ((k < 8))
            {
              {
                var l: dynamic = (k + 1);
                while ((l < 8))
                {
                  var v: dynamic = inter(lines[k], lines[l]);
                  if ((abs((v.x - 1000000009)) > EPS))
                  {
                    cons.push_back(v);
                  }
                  l += 1;
                }
              }
              k += 1;
            }
          }
          var ser: dynamic = cpp_uninitialized();
          {
            var k: dynamic = 0;
            while ((k < cons.size()))
            {
              var ok: dynamic = true;
              {
                var l: dynamic = 0;
                while ((l < 8))
                {
                  if (((l % 2) == 0))
                  {
                    if (((((cons[k].x * lines[l].a) + (cons[k].y * lines[l].b)) + EPS) < lines[l].c))
                    {
                      ok = false;
                    }
                  } else
                  {
                    if ((((cons[k].x * lines[l].a) + (cons[k].y * lines[l].b)) > (EPS + lines[l].c)))
                    {
                      ok = false;
                    }
                  }
                  l += 1;
                }
              }
              if (ok)
              {
                ser.push_back(cons[k]);
              }
              k += 1;
            }
          }
          if ((ser.size() >= 3))
          {
            var D: dynamic = convex_hull(ser);
            var S: dynamic = 0;
            {
              var k: dynamic = 0;
              while ((k < D.size()))
              {
                S += cross(D[k], D[(((k + 1)) % D.size())]);
                k += 1;
              }
            }
            ret += (S / 2);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%.12f\n", (ret / div));
}
