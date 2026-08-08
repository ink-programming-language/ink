// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var e: dynamic;

var f: dynamic;

func main()
{
  scanf("%d%d%d%d%d%d", (&a), (&b), (&c), (&d), (&e), (&f));
  var tmp1 = min(a, d);
  var tmp2 = min(b, min(c, d));
  var ans = 0;
  if ((e > f))
  {
    ans += (tmp1 * e);
    tmp2 = min(tmp2, (d - tmp1));
    ans += (tmp2 * f);
  } else
  {
    ans += (tmp2 * f);
    tmp1 = min(tmp1, (d - tmp2));
    ans += (tmp1 * e);
  }
  printf("%d", ans);
}
