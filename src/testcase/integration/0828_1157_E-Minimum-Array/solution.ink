// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200005);

var b: dynamic = cpp_array(200005);

var c: dynamic = cpp_array(200005);

var ans: dynamic = cpp_array(200005);

var t: dynamic = cpp_array((200005 << 3));

func pushup(x: dynamic) -> dynamic
{
  t[x] = (t[(x << 1)] + t[((x << 1) | 1)]);
}

func build(x: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    t[x] = c[l];
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  build((x << 1), l, mid);
  build(((x << 1) | 1), (mid + 1), r);
  pushup(x);
}

func update(x: dynamic, l: dynamic, r: dynamic, pos: dynamic, v: dynamic) -> dynamic
{
  if ((l == r))
  {
    t[x] -= v;
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  if ((pos <= mid))
  {
    update((x << 1), l, mid, pos, v);
  } else
  {
    update(((x << 1) | 1), (mid + 1), r, pos, v);
  }
  pushup(x);
}

func query(x: dynamic, l: dynamic, r: dynamic, lp: dynamic, rp: dynamic) -> dynamic
{
  if (((lp <= l) && (r <= rp)))
  {
    return t[x];
  }
  var mid: dynamic = (((l + r)) >> 1);
  var res: dynamic = 0;
  if ((lp <= mid))
  {
    res += query((x << 1), l, mid, lp, rp);
  }
  if ((mid < rp))
  {
    res += query(((x << 1) | 1), (mid + 1), r, lp, rp);
  }
  return res;
}

func calc(x: dynamic) -> dynamic
{
  var l: dynamic = x;
  var r: dynamic = n;
  if ((query(1, 1, n, l, r) == 0))
  {
    l = 1;
    r = (x - 1);
    var ans: dynamic = -1;
    while ((l <= r))
    {
      var mid: dynamic = (((l + r)) / 2);
      if ((query(1, 1, n, l, mid) > 0))
      {
        r = (mid - 1);
        ans = mid;
      } else
      {
        l = (mid + 1);
      }
    }
    return ans;
  } else
  {
    var ans: dynamic = -1;
    while ((l <= r))
    {
      var mid: dynamic = (((l + r)) / 2);
      if ((query(1, 1, n, l, mid) > 0))
      {
        r = (mid - 1);
        ans = mid;
      } else
      {
        l = (mid + 1);
      }
    }
    return ans;
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      c[(((b[i] % n)) + 1)] += 1;
      i += 1;
    }
  }
  build(1, 1, n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = (a[i] % n);
      var y: dynamic = (n - x);
      y += 1;
      var id: dynamic = calc(y);
      update(1, 1, n, id, 1);
      ans[i] = ((((x + id) - 1)) % n);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d%c", ans[i],  ((i == n)) ? cpp_char("\n") : cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
