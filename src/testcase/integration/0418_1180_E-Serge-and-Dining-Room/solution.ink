// Translated from solution.cpp.

var maxn: dynamic = 1e6;

var big: dynamic = 1e18;

var val: dynamic = cpp_array(maxn);

var t: dynamic = cpp_array(((4 * maxn) + 5));

var sum: dynamic = cpp_array(((4 * maxn) + 5));

func build(v: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((l == (r - 1)))
  {
    sum[v] = 0;
    t[v] = val[l];
    return;
  }
  var m: dynamic = (((l + r)) / 2);
  build((2 * v), l, m);
  build(((2 * v) + 1), m, r);
  t[v] = max(t[(2 * v)], t[((2 * v) + 1)]);
  sum[v] = 0;
}

func upd(v: dynamic, l: dynamic, r: dynamic, tl: dynamic, tr: dynamic, c: dynamic) -> dynamic
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
  var tm: dynamic = (((tl + tr)) / 2);
  upd((2 * v), l, r, tl, tm, c);
  upd(((2 * v) + 1), l, r, tm, tr, c);
  t[v] = max((t[(2 * v)] + sum[(2 * v)]), (t[((2 * v) + 1)] + sum[((2 * v) + 1)]));
}

func get(v: dynamic, l: dynamic, r: dynamic, tl: dynamic, tr: dynamic) -> dynamic
{
  if (((tr <= l) || (tl >= r)))
  {
    return (-big);
  }
  if (((tl >= l) && (tr <= r)))
  {
    return (t[v] + sum[v]);
  }
  var tm: dynamic = (((tl + tr)) / 2);
  return (max(get((2 * v), l, r, tl, tm), get(((2 * v) + 1), l, r, tm, tr)) + sum[v]);
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var a: dynamic = cpp_array(n);
  var b: dynamic = cpp_array(m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < maxn))
    {
      val[i] = 0;
      i += 1;
    }
  }
  build(1, 0, 1e6);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      upd(1, 0, a[i], 0, 1e6, 1);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      upd(1, 0, b[i], 0, 1e6, -1);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (q)
  {
    q -= 1;
    var type_cpp: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var id: dynamic = cpp_uninitialized();
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
    var l: dynamic = -1;
    var r: dynamic = 1e6;
    while ((l < (r - 1)))
    {
      var mid: dynamic = (((l + r)) / 2);
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  solve();
  return 0;
}
