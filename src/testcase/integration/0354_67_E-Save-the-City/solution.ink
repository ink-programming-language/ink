// Translated from solution.cpp.

var PI = acos(-1.0);

var eps = 1e-6;

class pos
{
  var x: dynamic;
  var y: dynamic;
}

class vec
{
  var x: dynamic;
  var y: dynamic;
}

class seg
{
  var a: dynamic;
  var b: dynamic;
}

func sign(x: dynamic)
{
  return if ((x < (-eps))) -1 else if ((x > eps)) 1 else 0;
}

func dot(a: dynamic, b: dynamic)
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func fwd(a: dynamic, b: dynamic)
{
  return [(b.x - a.x), (b.y - a.y)];
}

func mkang(src: dynamic, A: dynamic, B: dynamic)
{
}

func operator_add(p: dynamic, v: dynamic)
{
  return [(p.x + v.x), (p.y + v.y)];
}

func operator_multiply(v: dynamic, t: dynamic)
{
  return [(v.x * t), (v.y * t)];
}

func checkInt(a: dynamic, b: dynamic)
{
  return (sign(cross(fwd(a.a, a.b), fwd(b.a, b.b))) != 0);
}

func prt(p: dynamic)
{
  write("p(", p.x, ",", p.y, ") ");
}

func prt(p: dynamic)
{
  write("v(", p.x, ",", p.y, ") ");
}

func segIntSeg(a: dynamic, b: dynamic)
{
  var t = (cross(fwd(a.a, b.a), fwd(a.a, a.b)) / cross(fwd(a.a, a.b), fwd(b.a, b.b)));
  return (b.a + (fwd(b.a, b.b) * t));
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i].x, v[i].y);
      i += 1;
    }
  }
  var ln = [v[0], v[1]];
  var inv = (v[0].x > v[1].x);
  var L = if ((!inv)) v[0].x else v[1].x;
  var R = if ((!inv)) v[1].x else v[0].x;
  {
    var i = 2;
    while ((i < n))
    {
      {
        var j = 2;
        while ((j < i))
        {
          var ln2 = [v[i], v[j]];
          if ((!checkInt(ln, ln2)))
          {
            if ((((v[j].x < v[i].x)) ^ inv))
            {
              R = (L - 1);
            }
            j += 1;
            continue;
          }
          var p = segIntSeg(ln, ln2);
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
        var j = (i + 1);
        while ((j < n))
        {
          var ln2 = [v[i], v[j]];
          if ((!checkInt(ln, ln2)))
          {
            if ((((v[j].x > v[i].x)) ^ inv))
            {
              L = (R + 1);
            }
            j += 1;
            continue;
          }
          var p = segIntSeg(ln, ln2);
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
  var cnt = ((floor(R) - ceil(L)) + 1);
  write(max(cnt, 0), cpp_char("\n"));
}
