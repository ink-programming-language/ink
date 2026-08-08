// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(200005);

var b = cpp_array(200005);

var c = cpp_array(200005);

var ans = cpp_array(200005);

var t = cpp_array((200005 << 3));

func pushup(x: dynamic)
{
  t[x] = (t[(x << 1)] + t[((x << 1) | 1)]);
}

func build(x: dynamic, l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    t[x] = c[l];
    return;
  }
  var mid = (((l + r)) >> 1);
  build((x << 1), l, mid);
  build(((x << 1) | 1), (mid + 1), r);
  pushup(x);
}

func update(x: dynamic, l: dynamic, r: dynamic, pos: dynamic, v: dynamic)
{
  if ((l == r))
  {
    t[x] -= v;
    return;
  }
  var mid = (((l + r)) >> 1);
  if ((pos <= mid))
  {
    update((x << 1), l, mid, pos, v);
  } else
  {
    update(((x << 1) | 1), (mid + 1), r, pos, v);
  }
  pushup(x);
}

func query(x: dynamic, l: dynamic, r: dynamic, lp: dynamic, rp: dynamic)
{
  if (((lp <= l) && (r <= rp)))
  {
    return t[x];
  }
  var mid = (((l + r)) >> 1);
  var res = 0;
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

func calc(x: dynamic)
{
  var l = x;
  var r = n;
  if ((query(1, 1, n, l, r) == 0))
  {
    l = 1;
    r = (x - 1);
    var ans = -1;
    while ((l <= r))
    {
      var mid = (((l + r)) / 2);
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
    var ans = -1;
    while ((l <= r))
    {
      var mid = (((l + r)) / 2);
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

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      c[(((b[i] % n)) + 1)] += 1;
      i += 1;
    }
  }
  build(1, 1, n);
  {
    var i = 1;
    while ((i <= n))
    {
      var x = (a[i] % n);
      var y = (n - x);
      y += 1;
      var id = calc(y);
      update(1, 1, n, id, 1);
      ans[i] = ((((x + id) - 1)) % n);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d%c", ans[i], if ((i == n)) cpp_char("\n") else cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
