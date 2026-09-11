// Translated from solution.cpp.

var max_n: dynamic = (1e5 + 100);

var t: dynamic = cpp_array((4 * max_n));

var n: dynamic = cpp_uninitialized();

func max_on_seg(v: dynamic, tl: dynamic, tr: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (((tl == l) && (tr == r)))
  {
    return t[v];
  }
  var m: dynamic = (((tl + tr)) >> 1);
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

func update(v: dynamic, tl: dynamic, tr: dynamic, x: dynamic, d: dynamic) -> dynamic
{
  if ((tl == tr))
  {
    t[v] = d;
  } else
  {
    var m: dynamic = (((tl + tr)) >> 1);
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

func main() -> dynamic
{
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      read(a);
      var d: dynamic = (max_on_seg(1, 1, n, 1, a) + 1);
      ans = max(ans, d);
      update(1, 1, n, a, d);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
