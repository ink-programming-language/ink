// Translated from solution.cpp.

var maxn = cpp_expression("#inclu");

var maxm = cpp_expression("#i");

var ll = dynamic;

var inf = cpp_expression("#include<c");

var n: dynamic;

var q: dynamic;

var i: dynamic;

var j: dynamic;

var k: dynamic;

var id = cpp_array(maxn);

var L: dynamic;

class arr
{
  var w: dynamic;
  var v: dynamic;
  var c: dynamic;
  var i: dynamic;
}

var a = cpp_array(maxn);

func cmp(a: dynamic, b: dynamic)
{
  return ((a.v > b.v) || ((a.v == b.v) && (a.w < b.w)));
}

var res: dynamic;

var sum: dynamic;

var now: dynamic;

class val
{
  var w: dynamic;
  var v: dynamic;
}

func operator_add(a: dynamic, b: dynamic)
{
  return [(a.w + b.w), (a.v + b.v)];
}

func operator_multiply(a: dynamic, c: dynamic)
{
  return [(a.w * c), (a.v * c)];
}

func operator_less(a: dynamic, b: dynamic)
{
  return ((a.w < b.w) || ((a.w == b.w) && (a.v > b.v)));
}

class SegmentTree
{
  var lim: dynamic;
  var I: dynamic = cpp_array((maxn * 4));
  var ts: dynamic = cpp_array((maxn * 4));
  var tm: dynamic = cpp_array((maxn * 4));
  func newnode(x: dynamic, l: dynamic)
  {
      if ((a[l].w <= lim))
      {
        ts[x] = ([a[l].w, a[l].v] * a[l].c);
        tm[x] = [inf, 0];
        I[x] = 0;
      } else
      {
        ts[x] = [0, 0];
        if ((((a[l].w <= (lim << 1))) && a[l].c))
        {
          tm[x] = [a[l].w, a[l].v];
          I[x] = l;
        } else
        {
          tm[x] = [inf, 0];
          I[x] = 0;
        }
      }
    }
  func upd(x: dynamic)
  {
      ts[x] = (ts[(x << 1)] + ts[((x << 1) ^ 1)]);
      if ((tm[(x << 1)] < (ts[(x << 1)] + tm[((x << 1) ^ 1)])))
      {
        tm[x] = tm[(x << 1)];
        I[x] = I[(x << 1)];
      } else
      {
        tm[x] = (ts[(x << 1)] + tm[((x << 1) ^ 1)]);
        I[x] = I[((x << 1) ^ 1)];
      }
    }
  func maketree(x: dynamic, l: dynamic, r: dynamic)
  {
      if ((l == r))
      {
        newnode(x, l);
        return;
      }
      var mid = (((l + r)) >> 1);
      maketree((x << 1), l, mid);
      maketree(((x << 1) ^ 1), (mid + 1), r);
      upd(x);
    }
  func change(x: dynamic, l: dynamic, r: dynamic, p: dynamic)
  {
      if ((l == r))
      {
        newnode(x, l);
        return;
      }
      var mid = (((l + r)) >> 1);
      if ((p <= mid))
      {
        change((x << 1), l, mid, p);
      } else
      {
        change(((x << 1) ^ 1), (mid + 1), r, p);
      }
      upd(x);
    }
  func merge(x: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic)
  {
      if ((((l > R) || (r < L)) || (res < lim)))
      {
        return;
      }
      if (((L <= l) && (r <= R)))
      {
        if (((res >= tm[x].w) && I[x]))
        {
          if ((l == r))
          {
            res -= tm[x].w;
            sum += tm[x].v;
            now = l;
          } else
          {
            merge((x << 1), l, (((l + r)) >> 1), L, R);
            merge(((x << 1) ^ 1), (((((l + r)) >> 1)) + 1), r, L, R);
          }
          return;
        }
        if ((res >= ts[x].w))
        {
          res -= ts[x].w;
          sum += ts[x].v;
          now = r;
          return;
        } else if ((l == r))
        {
          var d = (res / a[l].w);
          res -= (d * a[l].w);
          sum += (d * a[l].v);
          now = l;
          return;
        }
      }
      var mid = (((l + r)) >> 1);
      merge((x << 1), l, mid, L, R);
      merge(((x << 1) ^ 1), (mid + 1), r, L, R);
    }
}

var t = cpp_array(maxm);

func main()
{
  scanf("%d%d", (&n), (&q));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%lld%lld%lld", (&a[i].c), (&a[i].w), (&a[i].v));
      a[i].i = i;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      k = max(k, cpp_cast(a[i].w));
      i += 1;
    }
  }
  sort((a + 1), ((a + 1) + n), cmp);
  {
    i = 1;
    while ((i <= n))
    {
      id[a[i].i] = i;
      i += 1;
    }
  }
  L = 0;
  while (((1 << L) < k))
  {
    L += 1;
  }
  {
    i = 0;
    while ((i <= L))
    {
      t[i].lim = (1 << i);
      t[i].maketree(1, 1, n);
      i += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    var tp: dynamic;
    scanf("%d", (&tp));
    if (((tp == 1) || (tp == 2)))
    {
      scanf("%d%d", (&j), (&k));
      a[id[k]].c += if (((tp == 1))) j else (-j);
      {
        i = 0;
        while ((i <= L))
        {
          t[i].change(1, 1, n, id[k]);
          i += 1;
        }
      }
    } else
    {
      sum = 0;
      now = 0;
      scanf("%lld", (&res));
      {
        i = L;
        while (((i >= 0) && (now < n)))
        {
          t[i].merge(1, 1, n, (now + 1), n);
          i -= 1;
        }
      }
      printf("%lld\n", sum);
    }
  }
}
