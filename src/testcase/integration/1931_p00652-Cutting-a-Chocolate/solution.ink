// Translated from solution.cpp.

func SZ(v: dynamic)
{
  return cpp_expression("#include <iostrea");
}

var maxint = (-1 >> 1);

var maxn = (30000 + 100);

var eps = 1e-8;

func sgn(x: dynamic)
{
  return (((x > eps)) - ((x < (-eps))));
}

class P
{
  var x: dynamic;
  var y: dynamic;
  func P()
  {
    }
  func P(x: dynamic, y: dynamic)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func cross(a: dynamic, b: dynamic)
  {
      return ((((a.x - x)) * ((b.y - y))) - (((a.y - y)) * ((b.x - x))));
    }
  func input()
  {
      scanf("%lf%lf", (&x), (&y));
    }
}

var n: dynamic;

var m: dynamic;

var w: dynamic;

var h: dynamic;

var s: dynamic;

var l = cpp_array(maxn);

var r = cpp_array(maxn);

var area = cpp_array(maxn);

var cnt = cpp_array(maxn);

func main()
{
  while (((scanf("%d%d%d%d%d", (&n), (&m), (&w), (&h), (&s)) == 5) && (((((n || m) || w) || h) || s))))
  {
    {
      var i = 1;
      while ((i <= m))
      {
        scanf("%d%d", (&l[i]), (&r[i]));
        i += 1;
      }
    }
    l[0] = cpp_assign(r[0], "=", 0);
    l[(m + 1)] = cpp_assign(r[(m + 1)], "=", h);
    {
      var i = 0;
      while ((i <= m))
      {
        area[i] = ((double((((r[(i + 1)] - r[i]) + l[(i + 1)]) - l[i])) * w) / 2);
        cnt[i] = 0;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        var pt: dynamic;
        pt.input();
        var low = 0;
        var high = m;
        var res = -1;
        while ((low <= high))
        {
          var mid = (((low + high)) / 2);
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
    var ba = 0;
    var now = 0;
    var ct = 0;
    var ans = 0;
    {
      var i = 0;
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
