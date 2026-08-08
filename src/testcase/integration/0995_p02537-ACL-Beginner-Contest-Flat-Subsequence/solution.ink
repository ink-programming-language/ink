// Translated from solution.cpp.

var MN = cpp_expression("#inclu");

var MA = cpp_expression("#inclu");

var n: dynamic;

var k: dynamic;

var ans = 0;

var T = [0];

func modify(k: dynamic, l: dynamic, r: dynamic, p: dynamic, w: dynamic)
{
  if ((l == r))
  {
    T[k] = w;
    return;
  }
  var mid = (((l + r)) >> 1);
  if ((p <= mid))
  {
    modify((k << 1), l, mid, p, w);
  } else
  {
    modify(((k << 1) | 1), (mid + 1), r, p, w);
  }
  T[k] = max(T[(k << 1)], T[((k << 1) | 1)]);
}

func query(k: dynamic, l: dynamic, r: dynamic, L: dynamic, R: dynamic)
{
  if (((l == L) && (r == R)))
  {
    return T[k];
  }
  var mid = (((l + r)) >> 1);
  if ((R <= mid))
  {
    return query((k << 1), l, mid, L, R);
  }
  if ((L > mid))
  {
    return query(((k << 1) | 1), (mid + 1), r, L, R);
  }
  return max(query((k << 1), l, mid, L, mid), query(((k << 1) | 1), (mid + 1), r, (mid + 1), R));
}

func main()
{
  scanf("%d%d", (&n), (&k));
  {
    var i = 1;
    var a: dynamic;
    while ((i <= n))
    {
      scanf("%d", (&a));
      var t = (query(1, 0, MA, max(0, (a - k)), min(MA, (a + k))) + 1);
      ans = max(ans, t);
      modify(1, 0, MA, a, t);
      i += 1;
    }
  }
  printf("%d\n", ans);
}
