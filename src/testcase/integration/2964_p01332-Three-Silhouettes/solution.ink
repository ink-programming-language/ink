// Translated from solution.cpp.

var EPS: dynamic = cpp_expression("#incl");

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(x: dynamic = 0.0, y: dynamic = 0.0) -> dynamic
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
  func operator_multiply(lambda: dynamic) -> dynamic
  {
      return Point((x * lambda), (y * lambda));
    }
  func operator_divide(lambda: dynamic) -> dynamic
  {
      return Point((x / lambda), (y / lambda));
    }
  func norm() -> dynamic
  {
      return ((x * x) + (y * y));
    }
  func abs() -> dynamic
  {
      return sqrt(norm());
    }
  func pol() -> dynamic
  {
      return atan2(y, x);
    }
  func operator_equal(p: dynamic) -> dynamic
  {
      return ((abs((x - p.x)) < EPS) && (abs((y - p.y)) < EPS));
    }
  func operator_less(p: dynamic) -> dynamic
  {
      if ((abs((x - p.x)) < EPS))
      {
        return (y < p.y);
      }
      return (x < p.x);
    }
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func det(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

class Segment
{
  var p1: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
  func Segment(p1: dynamic = Point(), p2: dynamic = Point()) -> dynamic
  {
      self->p1 = cpp_construct(p1);
      self->p2 = cpp_construct(p2);
    }
}

func crossPoint(a: dynamic, b: dynamic) -> dynamic
{
  var r: dynamic = (det((b.p2 - b.p1), (a.p1 - b.p1)) / det((a.p2 - a.p1), (b.p2 - b.p1)));
  return (a.p1 + (((a.p2 - a.p1)) * r));
}

func on_left(p: dynamic, l: dynamic) -> dynamic
{
  return (det((l.p2 - l.p1), (p - l.p1)) > EPS);
}

func para(a: dynamic, b: dynamic) -> dynamic
{
  return (abs(det((a.p2 - a.p1), (b.p2 - b.p1))) < EPS);
}

var L: dynamic = cpp_array(15);

var que: dynamic = cpp_array(15);

var he: dynamic = cpp_uninitialized();

var ta: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (((a.p2 - a.p1)).pol() < ((b.p2 - b.p1)).pol());
}

func area(P: dynamic) -> dynamic
{
  var ans: dynamic = 0.0;
  var num: dynamic = P.size();
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      ans += det(P[i], P[(((i + 1)) % num)]);
      i += 1;
    }
  }
  return (ans / 2.0);
}

func hp_intersect() -> dynamic
{
  sort((L + 1), ((L + n) + 1), cmp);
  he = cpp_assign(ta, "=", 0);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      while ((((ta - he) > 1) && (!on_left(crossPoint(que[(ta - 1)], que[(ta - 2)]), L[i]))))
      {
        ta -= 1;
      }
      while ((((ta - he) > 1) && (!on_left(crossPoint(que[he], que[(he + 1)]), L[i]))))
      {
        he += 1;
      }
      que[cpp_update(ta, "++")] = L[i];
      while ((((ta - he) > 1) && para(que[(ta - 1)], que[(ta - 2)])))
      {
        ta -= 1;
        if (on_left(L[i].p1, que[(ta - 1)]))
        {
          que[(ta - 1)] = L[i];
        }
      }
      i += 1;
    }
  }
  while ((((ta - he) > 1) && (!on_left(crossPoint(que[(ta - 1)], que[(ta - 2)]), que[he]))))
  {
    ta -= 1;
  }
  if (((ta - he) <= 2))
  {
    return 0.0;
  }
  var cnt: dynamic = (ta - he);
  {
    var i: dynamic = 0;
    while ((i < cnt))
    {
      po[i] = crossPoint(que[(he + i)], que[(he + (((i + 1)) % cnt))]);
      i += 1;
    }
  }
  return area(po);
}

func cut(P: dynamic, x0: dynamic, lis: dynamic) -> dynamic
{
  lis.clear();
  var num: dynamic = P.size();
  var l: dynamic = Line(Point(x0, -1.0), Point(x0, 301.0));
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      if ((P[i].x == P[(((i + 1)) % num)].x))
      {
        i += 1;
        continue;
      }
      if (((((P[i].x - x0)) * ((P[(((i + 1)) % num)].x - x0))) < 0))
      {
        lis.push_back(crossPoint(Line(P[i], P[(((i + 1)) % num)]), l).y);
      }
      i += 1;
    }
  }
  sort(lis.begin(), lis.end());
}

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

var Z: dynamic = cpp_uninitialized();

var nx: dynamic = cpp_uninitialized();

var ny: dynamic = cpp_uninitialized();

var nz: dynamic = cpp_uninitialized();

var ylis: dynamic = cpp_uninitialized();

var zlis: dynamic = cpp_uninitialized();

func calc(x0: dynamic) -> dynamic
{
  cut(Y, x0, ylis);
  cut(Z, x0, zlis);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(ylis.size())))
    {
      {
        var j: dynamic = 0;
        while ((j < cpp_cast(zlis.size())))
        {
          {
            var k: dynamic = 0;
            while ((k < nx))
            {
              n = 0;
              L[cpp_update(n, "++")] = Line(Point(zlis[j], ylis[i]), Point(zlis[(j + 1)], ylis[i]));
              L[cpp_update(n, "++")] = Line(Point(zlis[(j + 1)], ylis[i]), Point(zlis[(j + 1)], ylis[(i + 1)]));
              L[cpp_update(n, "++")] = Line(Point(zlis[(j + 1)], ylis[(i + 1)]), Point(zlis[j], ylis[(i + 1)]));
              L[cpp_update(n, "++")] = Line(Point(zlis[j], ylis[(i + 1)]), Point(zlis[j], ylis[i]));
              if ((det((X[k] + Point(1, 1)), (X[(((k + 1)) % nx)] + Point(1, 1))) > 0))
              {
                L[cpp_update(n, "++")] = Line(Point(-1, -1), X[k]);
                L[cpp_update(n, "++")] = Line(X[k], X[(((k + 1)) % nx)]);
                L[cpp_update(n, "++")] = Line(X[(((k + 1)) % nx)], Point(-1, -1));
                ans += hp_intersect();
              } else
              {
                L[cpp_update(n, "++")] = Line(Point(-1, -1), X[(((k + 1)) % nx)]);
                L[cpp_update(n, "++")] = Line(X[(((k + 1)) % nx)], X[k]);
                L[cpp_update(n, "++")] = Line(X[k], Point(-1, -1));
                ans -= hp_intersect();
              }
              k += 1;
            }
          }
          j += 2;
        }
      }
      i += 2;
    }
  }
  return ans;
}

func work(l: dynamic, r: dynamic, y_l: dynamic, y_mid: dynamic, y_r: dynamic, eps: dynamic) -> dynamic
{
  var mid: dynamic = (((l + r)) / 2);
  var y_lmid: dynamic = calc((((l + mid)) / 2));
  var y_rmid: dynamic = calc((((mid + r)) / 2));
  var s: dynamic = ((((r - l)) * (((y_l + (4 * y_mid)) + y_r))) / 6);
  var s1: dynamic = ((((mid - l)) * (((y_l + (4 * y_lmid)) + y_mid))) / 6);
  var s2: dynamic = ((((r - mid)) * (((y_mid + (4 * y_rmid)) + y_r))) / 6);
  if ((abs(((s1 + s2) - s)) < (15 * eps)))
  {
    return ((s1 + s2) + ((((s1 + s2) - s)) / 15));
  }
  return (work(l, mid, y_l, y_lmid, y_mid, (eps / 2)) + work(mid, r, y_mid, y_rmid, y_r, (eps / 2)));
}

var xs: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  sort(xs.begin(), xs.end());
  if ((t == 20))
  {
    printf("%.6f\n", work((xs.front() + EPS), (xs.back() - EPS), calc((xs.front() + EPS)), calc((((xs.front() + xs.back())) / 2)), calc((xs.back() - EPS)), 1e-7));
    return;
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast(xs.size()) - 1)))
    {
      if (((xs[i] + 1e-11) < (xs[(i + 1)] - 1e-11)))
      {
        ans += work((xs[i] + 1e-11), (xs[(i + 1)] - 1e-11), calc((xs[i] + 1e-11)), calc((((xs[i] + xs[(i + 1)])) / 2)), calc((xs[(i + 1)] - 1e-11)), 1e-9);
      }
      i += 1;
    }
  }
  printf("%.6f\n", ans);
}

func main() -> dynamic
{
  while (((~scanf("%d", (&nx))) && nx))
  {
    t += 1;
    X.resize(nx);
    {
      var i: dynamic = 0;
      while ((i < nx))
      {
        scanf("%lf%lf", (&X[i].x), (&X[i].y));
        i += 1;
      }
    }
    scanf("%d", (&ny));
    Y.resize(ny);
    {
      var i: dynamic = 0;
      while ((i < ny))
      {
        scanf("%lf%lf", (&Y[i].y), (&Y[i].x));
        xs.push_back(Y[i].x);
        i += 1;
      }
    }
    reverse(Y.begin(), Y.end());
    scanf("%d", (&nz));
    Z.resize(nz);
    {
      var i: dynamic = 0;
      while ((i < nz))
      {
        scanf("%lf%lf", (&Z[i].x), (&Z[i].y));
        xs.push_back(Z[i].x);
        i += 1;
      }
    }
    solve();
  }
  return 0;
}
