// Translated from solution.cpp.

var N = 200005;

var c = cpp_array(N);

class Node
{
  var l: dynamic;
  var r: dynamic;
  var v: dynamic;
  var cv: dynamic;
}

var a = cpp_array((N << 2));

func Build(i: dynamic, l: dynamic, r: dynamic)
{
  a[i].l = l;
  a[i].r = r;
  a[i].cv = false;
  if ((l == r))
  {
    return;
  }
  var mid = ((l + r) >> 1);
  Build((i << 1), l, mid);
  Build(((i << 1) | 1), (mid + 1), r);
}

func PushUp(i: dynamic)
{
  a[i].v = (a[(i << 1)].v + a[((i << 1) | 1)].v);
}

func PushDown(i: dynamic)
{
  if (a[i].cv)
  {
    a[(i << 1)].cv = cpp_assign(a[((i << 1) | 1)].cv, "=", a[i].cv);
    a[(i << 1)].v = cpp_assign(a[((i << 1) | 1)].v, "=", 0);
    a[i].cv = false;
  }
}

func Mdf(i: dynamic, x: dynamic, val: dynamic)
{
  if (((a[i].l == x) && (a[i].r == x)))
  {
    a[i].v = val;
    return;
  }
  PushDown(i);
  if ((x <= a[(i << 1)].r))
  {
    Mdf((i << 1), x, val);
  } else
  {
    Mdf(((i << 1) | 1), x, val);
  }
  PushUp(i);
}

func Cv(i: dynamic, l: dynamic, r: dynamic)
{
  if (((l <= a[i].l) && (a[i].r <= r)))
  {
    a[i].cv = true;
    a[i].v = 0;
    return;
  }
  PushDown(i);
  if ((l <= a[(i << 1)].r))
  {
    Cv((i << 1), l, r);
  }
  if ((a[((i << 1) | 1)].l <= r))
  {
    Cv(((i << 1) | 1), l, r);
  }
  PushUp(i);
}

func Qry(i: dynamic, l: dynamic, r: dynamic)
{
  if (((l <= a[i].l) && (a[i].r <= r)))
  {
    return a[i].v;
  }
  PushDown(i);
  if ((r <= a[(i << 1)].r))
  {
    return Qry((i << 1), l, r);
  } else if ((a[((i << 1) | 1)].l <= l))
  {
    return Qry(((i << 1) | 1), l, r);
  } else
  {
    return (Qry((i << 1), l, r) + Qry(((i << 1) | 1), l, r));
  }
}

class Query
{
  var l: dynamic;
  var r: dynamic;
  var id: dynamic;
}

var q = cpp_array(N);

var ans = cpp_array(N);

func cmp(q: dynamic, w: dynamic)
{
  return (q.l > w.l);
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  Build(1, 1, n);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d %d", (&c[i].first), (&c[i].second));
      i += 1;
    }
  }
  c[(n + 1)].first = c[n].second;
  var m: dynamic;
  scanf("%d", (&m));
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d %d", (&q[i].l), (&q[i].r));
      q[i].id = i;
      i += 1;
    }
  }
  sort((q + 1), ((q + 1) + m), cmp);
  var pos = 1;
  {
    var i = n;
    while ((i >= 1))
    {
      if (((c[i].first + c[i].second) >= c[(i + 1)].first))
      {
        var l = (i + 1);
        var r = n;
        while ((l < r))
        {
          var mid = (((l + r) + 1) >> 1);
          if (((c[i].first + c[i].second) >= c[mid].first))
          {
            l = mid;
          } else
          {
            r = (mid - 1);
          }
        }
        Cv(1, i, l);
        if ((l < n))
        {
          Mdf(1, (l + 1), min(Qry(1, (l + 1), (l + 1)), ((c[(l + 1)].first - c[i].first) - c[i].second)));
        }
      } else
      {
        Mdf(1, (i + 1), ((c[(i + 1)].first - c[i].first) - c[i].second));
      }
      while ((q[pos].l == i))
      {
        ans[q[pos].id] = Qry(1, 1, q[pos].r);
        pos += 1;
      }
      if ((pos > m))
      {
        break;
      }
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
