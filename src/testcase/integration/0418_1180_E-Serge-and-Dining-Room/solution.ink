// Translated from solution.cpp.

var maxn = 1e6;

var big = 1e18;

var val = cpp_array(maxn);

var t = cpp_array(((4 * maxn) + 5));

var sum = cpp_array(((4 * maxn) + 5));

func build(v: dynamic, l: dynamic, r: dynamic)
{
  if ((l == (r - 1)))
  {
    sum[v] = 0;
    t[v] = val[l];
    return;
  }
  var m = (((l + r)) / 2);
  build((2 * v), l, m);
  build(((2 * v) + 1), m, r);
  t[v] = max(t[(2 * v)], t[((2 * v) + 1)]);
  sum[v] = 0;
}

func upd(v: dynamic, l: dynamic, r: dynamic, tl: dynamic, tr: dynamic, c: dynamic)
{
  if (((tr <= l) || (tl >= r)))
  {
    return;
  }
  if (((tl >= l) && (tr <= r)))
  {
    sum[v] += c;
    return;
  }
  var tm = (((tl + tr)) / 2);
  upd((2 * v), l, r, tl, tm, c);
  upd(((2 * v) + 1), l, r, tm, tr, c);
  t[v] = max((t[(2 * v)] + sum[(2 * v)]), (t[((2 * v) + 1)] + sum[((2 * v) + 1)]));
}

func get(v: dynamic, l: dynamic, r: dynamic, tl: dynamic, tr: dynamic)
{
  if (((tr <= l) || (tl >= r)))
  {
    return (-big);
  }
  if (((tl >= l) && (tr <= r)))
  {
    return (t[v] + sum[v]);
  }
  var tm = (((tl + tr)) / 2);
  return (max(get((2 * v), l, r, tl, tm), get(((2 * v) + 1), l, r, tm, tr)) + sum[v]);
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a = cpp_array(n);
  var b = cpp_array(m);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < maxn))
    {
      val[i] = 0;
      i += 1;
    }
  }
  build(1, 0, 1e6);
  {
    var i = 0;
    while ((i < n))
    {
      upd(1, 0, a[i], 0, 1e6, 1);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      upd(1, 0, b[i], 0, 1e6, -1);
      i += 1;
    }
  }
  var q: dynamic;
  read(q);
  while (q)
  {
    q -= 1;
    var type_cpp: dynamic;
    var v: dynamic;
    var id: dynamic;
    read(type_cpp, id, v);
    id -= 1;
    if ((type_cpp == 1))
    {
      upd(1, 0, a[id], 0, 1e6, -1);
      a[id] = v;
      upd(1, 0, a[id], 0, 1e6, 1);
    } else
    {
      upd(1, 0, b[id], 0, 1e6, 1);
      b[id] = v;
      upd(1, 0, b[id], 0, 1e6, -1);
    }
    var l = -1;
    var r = 1e6;
    while ((l < (r - 1)))
    {
      var mid = (((l + r)) / 2);
      if ((get(1, mid, 1e6, 0, 1e6) > 0))
      {
        l = mid;
      } else
      {
        r = mid;
      }
    }
    if ((l == -1))
    {
      write("-1\n");
    } else
    {
      write((l + 1), cpp_char("\n"));
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  solve();
  return 0;
}
