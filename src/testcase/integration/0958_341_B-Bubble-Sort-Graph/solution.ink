// Translated from solution.cpp.

var max_n = (1e5 + 100);

var t = cpp_array((4 * max_n));

var n: dynamic;

func max_on_seg(v: dynamic, tl: dynamic, tr: dynamic, l: dynamic, r: dynamic)
{
  if (((tl == l) && (tr == r)))
  {
    return t[v];
  }
  var m = (((tl + tr)) >> 1);
  if ((r <= m))
  {
    return max_on_seg((2 * v), tl, m, l, r);
  }
  if ((l > m))
  {
    return max_on_seg(((2 * v) + 1), (m + 1), tr, l, r);
  }
  return max(max_on_seg((2 * v), tl, m, l, m), max_on_seg(((2 * v) + 1), (m + 1), tr, (m + 1), r));
}

func update(v: dynamic, tl: dynamic, tr: dynamic, x: dynamic, d: dynamic)
{
  if ((tl == tr))
  {
    t[v] = d;
  } else
  {
    var m = (((tl + tr)) >> 1);
    if ((x <= m))
    {
      update((2 * v), tl, m, x, d);
    } else
    {
      update(((2 * v) + 1), (m + 1), tr, x, d);
    }
    t[v] = max(t[(2 * v)], t[((2 * v) + 1)]);
  }
}

func main()
{
  read(n);
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      read(a);
      var d = (max_on_seg(1, 1, n, 1, a) + 1);
      ans = max(ans, d);
      update(1, 1, n, a, d);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
