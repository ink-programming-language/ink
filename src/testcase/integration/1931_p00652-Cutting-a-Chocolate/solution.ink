// Translated from solution.cpp.

func SZ(v: dynamic) -> dynamic
{
  return cpp_expression("#include <iostrea");
}

var maxint: dynamic = (-1 >> 1);

var maxn: dynamic = (30000 + 100);

var eps: dynamic = 1e-8;

func sgn(x: dynamic) -> dynamic
{
  return (((x > eps)) - ((x < (-eps))));
}

class P
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func P() -> dynamic
  {
    }
  func P(x: dynamic, y: dynamic) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func cross(a: dynamic, b: dynamic) -> dynamic
  {
      return ((((a.x - x)) * ((b.y - y))) - (((a.y - y)) * ((b.x - x))));
    }
  func input() -> dynamic
  {
      scanf("%lf%lf", (&x), (&y));
    }
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var l: dynamic = cpp_array(maxn);

var r: dynamic = cpp_array(maxn);

var area: dynamic = cpp_array(maxn);

var cnt: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  while (((scanf("%d%d%d%d%d", (&n), (&m), (&w), (&h), (&s)) == 5) && (((((n || m) || w) || h) || s))))
  {
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        scanf("%d%d", (&l[i]), (&r[i]));
        i += 1;
      }
    }
    l[0] = cpp_assign(r[0], "=", 0);
    l[(m + 1)] = cpp_assign(r[(m + 1)], "=", h);
    {
      var i: dynamic = 0;
      while ((i <= m))
      {
        area[i] = ((cpp_double((((r[(i + 1)] - r[i]) + l[(i + 1)]) - l[i])) * w) / 2);
        cnt[i] = 0;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var pt: dynamic = cpp_uninitialized();
        pt.input();
        var low: dynamic = 0;
        var high: dynamic = m;
        var res: dynamic = -1;
        while ((low <= high))
        {
          var mid: dynamic = (((low + high)) / 2);
          if ((sgn(P(0, l[mid]).cross(P(w, r[mid]), pt)) > 0))
          {
            res = mid;
            low = (mid + 1);
          } else
          {
            high = (mid - 1);
          }
        }
        if ((res == -1))
        {
          while (1)
          {
          }
        }
        if ((res != -1))
        {
          cnt[res] += 1;
        }
        i += 1;
      }
    }
    var ba: dynamic = 0;
    var now: dynamic = 0;
    var ct: dynamic = 0;
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i <= m))
      {
        now += area[i];
        ct += cnt[i];
        while (((ba <= i) && (sgn(((now - (w * h)) + s)) > 0)))
        {
          now -= area[ba];
          ct -= cnt[ba];
          ba += 1;
        }
        ans = max(ans, ct);
        i += 1;
      }
    }
    printf("%d\n", (n - ans));
  }
  return 0;
}
