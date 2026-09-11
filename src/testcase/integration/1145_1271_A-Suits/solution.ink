// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

var f: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%d%d%d%d%d", (&a), (&b), (&c), (&d), (&e), (&f));
  var tmp1: dynamic = min(a, d);
  var tmp2: dynamic = min(b, min(c, d));
  var ans: dynamic = 0;
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
