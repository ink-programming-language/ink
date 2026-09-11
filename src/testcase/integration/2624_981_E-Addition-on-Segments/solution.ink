// Translated from solution.cpp.

var mt_rand: dynamic = cpp_construct(chrono.system_clock.now().time_since_epoch().count());

func upmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func upmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

var maxn: dynamic = cpp_cast(10002);

var base: dynamic = 2147483647;

var PI: dynamic = acos(-1.0);

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array((4 * maxn));

func upd(v: dynamic, tl: dynamic, tr: dynamic, l: dynamic, r: dynamic, x: dynamic) -> dynamic
{
  if (((l == tl) && (r == tr)))
  {
    t[v].push_back(x);
  } else
  {
    var m: dynamic = (((tl + tr)) >> 1);
    if ((l <= m))
    {
      upd((v + v), tl, m, l, min(r, m), x);
    }
    if ((r > m))
    {
      upd(((v + v) + 1), (m + 1), tr, max(l, (m + 1)), r, x);
    }
  }
}

func getAns(v: dynamic, l: dynamic, r: dynamic, cur: dynamic) -> dynamic
{
  for (var x: dynamic in t[v])
  {
    cur = (((cur << x)) | cur);
  }
  ans |= cur;
  if ((l == r))
  {
    return;
  }
  var m: dynamic = (((l + r)) >> 1);
  getAns((v + v), l, m, cur);
  getAns(((v + v) + 1), (m + 1), r, cur);
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&q));
  a.set(0);
  ans.set(0);
  var event: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&l), (&r), (&x));
      upd(1, 1, n, l, r, x);
      i += 1;
    }
  }
  getAns(1, 1, n, a);
  var vec: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (ans[i])
      {
        vec.push_back(i);
      }
      i += 1;
    }
  }
  write(vec.size(), cpp_char("\n"));
  for (var v: dynamic in vec)
  {
    write(v, " ");
  }
  return 0;
}
