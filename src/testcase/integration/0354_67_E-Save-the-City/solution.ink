// Translated from solution.cpp.

var PI: dynamic = acos(-1.0);

var eps: dynamic = 1e-6;

class pos
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

class vec
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

class seg
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

func sign(x: dynamic) -> dynamic
{
  return  ((x < (-eps))) ? -1 :  ((x > eps)) ? 1 : 0;
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func fwd(a: dynamic, b: dynamic) -> dynamic
{
  return [(b.x - a.x), (b.y - a.y)];
}

func mkang(src: dynamic, A: dynamic, B: dynamic) -> dynamic
{
}

func operator_add(p: dynamic, v: dynamic) -> dynamic
{
  return [(p.x + v.x), (p.y + v.y)];
}

func operator_multiply(v: dynamic, t: dynamic) -> dynamic
{
  return [(v.x * t), (v.y * t)];
}

func checkInt(a: dynamic, b: dynamic) -> dynamic
{
  return (sign(cross(fwd(a.a, a.b), fwd(b.a, b.b))) != 0);
}

func prt(p: dynamic) -> dynamic
{
  write("p(", p.x, ",", p.y, ") ");
}

func prt(p: dynamic) -> dynamic
{
  write("v(", p.x, ",", p.y, ") ");
}

func segIntSeg(a: dynamic, b: dynamic) -> dynamic
{
  var t: dynamic = (cross(fwd(a.a, b.a), fwd(a.a, a.b)) / cross(fwd(a.a, a.b), fwd(b.a, b.b)));
  return (b.a + (fwd(b.a, b.b) * t));
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i].x, v[i].y);
      i += 1;
    }
  }
  var ln: dynamic = [v[0], v[1]];
  var inv: dynamic = (v[0].x > v[1].x);
  var L: dynamic =  ((!inv)) ? v[0].x : v[1].x;
  var R: dynamic =  ((!inv)) ? v[1].x : v[0].x;
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      {
        var j: dynamic = 2;
        while ((j < i))
        {
          var ln2: dynamic = [v[i], v[j]];
          if ((!checkInt(ln, ln2)))
          {
            if ((((v[j].x < v[i].x)) ^ inv))
            {
              R = (L - 1);
            }
            j += 1;
            continue;
          }
          var p: dynamic = segIntSeg(ln, ln2);
          if ((sign(dot(fwd(v[i], v[j]), fwd(v[i], p))) < 0))
          {
            j += 1;
            continue;
          }
          if ((!inv))
          {
            R = min(R, p.x);
          } else
          {
            L = max(L, p.x);
          }
          j += 1;
        }
      }
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          var ln2: dynamic = [v[i], v[j]];
          if ((!checkInt(ln, ln2)))
          {
            if ((((v[j].x > v[i].x)) ^ inv))
            {
              L = (R + 1);
            }
            j += 1;
            continue;
          }
          var p: dynamic = segIntSeg(ln, ln2);
          if ((sign(dot(fwd(v[i], v[j]), fwd(v[i], p))) < 0))
          {
            j += 1;
            continue;
          }
          if ((!inv))
          {
            L = max(L, p.x);
          } else
          {
            R = min(R, p.x);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var cnt: dynamic = ((floor(R) - ceil(L)) + 1);
  write(max(cnt, 0), cpp_char("\n"));
}
