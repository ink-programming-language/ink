// Translated from solution.cpp.

var mt_rand = cpp_construct(chrono.system_clock.now().time_since_epoch().count());

func upmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func upmin(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

var maxn = cpp_cast(10002);

var base = 2147483647;

var PI = acos(-1.0);

var n: dynamic;

var q: dynamic;

var a: dynamic;

var ans: dynamic;

var t = cpp_array((4 * maxn));

func upd(v: dynamic, tl: dynamic, tr: dynamic, l: dynamic, r: dynamic, x: dynamic)
{
  if (((l == tl) && (r == tr)))
  {
    t[v].push_back(x);
  } else
  {
    var m = (((tl + tr)) >> 1);
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

func getAns(v: dynamic, l: dynamic, r: dynamic, cur: dynamic)
{
  for (var x in t[v])
  {
    cur = (((cur << x)) | cur);
  }
  ans |= cur;
  if ((l == r))
  {
    return;
  }
  var m = (((l + r)) >> 1);
  getAns((v + v), l, m, cur);
  getAns(((v + v) + 1), (m + 1), r, cur);
}

func main()
{
  scanf("%d%d", (&n), (&q));
  a.set(0);
  ans.set(0);
  var event: dynamic;
  {
    var i = 1;
    while ((i <= q))
    {
      var l: dynamic;
      var r: dynamic;
      var x: dynamic;
      scanf("%d%d%d", (&l), (&r), (&x));
      upd(1, 1, n, l, r, x);
      i += 1;
    }
  }
  getAns(1, 1, n, a);
  var vec: dynamic;
  {
    var i = 1;
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
  for (var v in vec)
  {
    write(v, " ");
  }
  return 0;
}
