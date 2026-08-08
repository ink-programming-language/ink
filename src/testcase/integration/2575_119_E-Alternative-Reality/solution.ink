// Translated from solution.cpp.

func convert(x: dynamic)
{
  var ss: dynamic;
  (ss << x);
  var ret: dynamic;
  (ss >> ret);
  return ret;
}

var oo = ((~0) >> 2);

var eps = 1e-10;

var mn = 1000;

var mo = 100000007;

var fx = [[-1, 0], [1, 0], [0, -1], [0, 1], [-1, -1], [-1, 1], [1, -1], [1, 1]];

class po
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  func po()
  {
    }
  func po(X: dynamic, Y: dynamic)
  {
      x = X;
      y = Y;
      z = 0;
    }
  func po(X: dynamic, Y: dynamic, Z: dynamic)
  {
      x = X;
      y = Y;
      z = Z;
    }
  func operator_divide(a: dynamic)
  {
      return po((x / a), (y / a), (z / a));
    }
  func operator_add(p: dynamic)
  {
      return po((x + p.x), (y + p.y), (z + p.z));
    }
  func operator_subtract(p: dynamic)
  {
      return po((x - p.x), (y - p.y), (z - p.z));
    }
}

var a = cpp_array(mn);

var p = cpp_array(mn);

var n: dynamic;

var m: dynamic;

func sgn(x: dynamic)
{
  if ((fabs(x) <= eps))
  {
    return 0;
  }
  if ((x > (-eps)))
  {
    return 1;
  }
  return -1;
}

func dis(a: dynamic, b: dynamic)
{
  return sqrt(((((cpp_cast(((a.x - b.x))) * ((a.x - b.x)))) + ((cpp_cast(((a.y - b.y))) * ((a.y - b.y))))) + ((cpp_cast(((a.z - b.z))) * ((a.z - b.z))))));
}

func Project(p: dynamic, A: dynamic, B: dynamic, C: dynamic)
{
  if (((!A) && (!B)))
  {
    return po(p.x, p.y);
  }
  if (((!B) && (!C)))
  {
    return po(p.y, p.z);
  }
  if (((!A) && (!C)))
  {
    return po(p.z, p.x);
  }
  var alpha = (cpp_cast(((((A * p.x) + (B * p.y)) + (C * p.z)))) / (((((cpp_cast((A)) * (A))) + ((cpp_cast((B)) * (B)))) + ((cpp_cast((C)) * (C))))));
  var pA = po((p.x - (alpha * A)), (p.y - (alpha * B)), (p.z - (alpha * C)));
  var pO = po(0, 0);
  var pB = po(0, (-C), B);
  var a = dis(pO, pA);
  var b = dis(pO, pB);
  var c = dis(pA, pB);
  if ((!sgn(a)))
  {
    return po(0, 0);
  }
  var t = ((((((cpp_cast((a)) * (a))) + ((cpp_cast((b)) * (b)))) - ((cpp_cast((c)) * (c))))) / (((2 * a) * b)));
  var Ang: dynamic;
  if ((!sgn((t - 1))))
  {
    Ang = acos(1);
  } else if ((!sgn((t + 1))))
  {
    Ang = acos(-1);
  } else
  {
    Ang = acos(t);
  }
  if ((sgn(pA.x) < 0))
  {
    Ang *= -1;
  }
  return po((a * cos(Ang)), (a * sin(Ang)));
}

func calc(A: dynamic, B: dynamic, C: dynamic)
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var e: dynamic;
  var f: dynamic;
  a = (B.x - A.x);
  b = (B.y - A.y);
  c = (((((((cpp_cast((B.x)) * (B.x))) + ((cpp_cast((B.y)) * (B.y)))) - ((cpp_cast((A.x)) * (A.x)))) - ((cpp_cast((A.y)) * (A.y))))) / 2.0);
  d = (C.x - A.x);
  e = (C.y - A.y);
  f = (((((((cpp_cast((C.x)) * (C.x))) + ((cpp_cast((C.y)) * (C.y)))) - ((cpp_cast((A.x)) * (A.x)))) - ((cpp_cast((A.y)) * (A.y))))) / 2.0);
  return po(((((c * e) - (f * b))) / (((a * e) - (b * d)))), ((((c * d) - (f * a))) / (((b * d) - (e * a)))));
}

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].x, a[i].y, a[i].z);
      i += 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    var A: dynamic;
    var B: dynamic;
    var C: dynamic;
    read(A, B, C);
    {
      var i = 1;
      while ((i <= n))
      {
        p[i] = Project(a[i], A, B, C);
        i += 1;
      }
    }
    random_shuffle((p + 1), ((p + n) + 1));
    var o = po(0, 0, 0);
    var R = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        if ((sgn((dis(o, p[i]) - R)) > 0))
        {
          o = p[i];
          R = 0;
          {
            var j = 1;
            while ((j <= (i - 1)))
            {
              if ((sgn((dis(o, p[j]) - R)) > 0))
              {
                o = (((p[i] + p[j])) / 2);
                R = dis(o, p[i]);
                {
                  var k = 1;
                  while ((k <= (j - 1)))
                  {
                    if ((sgn((dis(o, p[k]) - R)) > 0))
                    {
                      o = calc(p[i], p[j], p[k]);
                      R = dis(o, p[i]);
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
    printf("%.10f\n", R);
  }
}
