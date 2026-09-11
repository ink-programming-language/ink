// Translated from solution.cpp.

var maxn: dynamic = cpp_expression("#inclu");

var maxm: dynamic = cpp_expression("#i");

var ll: dynamic = dynamic;

var inf: dynamic = cpp_expression("#include<c");

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var id: dynamic = cpp_array(maxn);

var L: dynamic = cpp_uninitialized();

class arr
{
  var w: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(maxn);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.v > b.v) || ((a.v == b.v) && (a.w < b.w)));
}

var res: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var now: dynamic = cpp_uninitialized();

class val
{
  var w: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return [(a.w + b.w), (a.v + b.v)];
}

func operator_multiply(a: dynamic, c: dynamic) -> dynamic
{
  return [(a.w * c), (a.v * c)];
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.w < b.w) || ((a.w == b.w) && (a.v > b.v)));
}

class SegmentTree
{
  var lim: dynamic = cpp_uninitialized();
  var I: dynamic = cpp_array((maxn * 4));
  var ts: dynamic = cpp_array((maxn * 4));
  var tm: dynamic = cpp_array((maxn * 4));
  func newnode(x: dynamic, l: dynamic) -> dynamic
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
  func upd(x: dynamic) -> dynamic
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
  func maketree(x: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if ((l == r))
      {
        newnode(x, l);
        return;
      }
      var mid: dynamic = (((l + r)) >> 1);
      maketree((x << 1), l, mid);
      maketree(((x << 1) ^ 1), (mid + 1), r);
      upd(x);
    }
  func change(x: dynamic, l: dynamic, r: dynamic, p: dynamic) -> dynamic
  {
      if ((l == r))
      {
        newnode(x, l);
        return;
      }
      var mid: dynamic = (((l + r)) >> 1);
      if ((p <= mid))
      {
        change((x << 1), l, mid, p);
      } else
      {
        change(((x << 1) ^ 1), (mid + 1), r, p);
      }
      upd(x);
    }
  func merge(x: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic) -> dynamic
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
          var d: dynamic = (res / a[l].w);
          res -= (d * a[l].w);
          sum += (d * a[l].v);
          now = l;
          return;
        }
      }
      var mid: dynamic = (((l + r)) >> 1);
      merge((x << 1), l, mid, L, R);
      merge(((x << 1) ^ 1), (mid + 1), r, L, R);
    }
}

var t: dynamic = cpp_array(maxm);

func main() -> dynamic
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
    var tp: dynamic = cpp_uninitialized();
    scanf("%d", (&tp));
    if (((tp == 1) || (tp == 2)))
    {
      scanf("%d%d", (&j), (&k));
      a[id[k]].c +=  (((tp == 1))) ? j : (-j);
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
