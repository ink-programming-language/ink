// Translated from solution.cpp.

var K: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%I64d%I64d%I64d", (&K), (&d), (&t));
  d = ((((((K - 1)) / d) + 1)) * d);
  t = (t * 2);
  var now: dynamic = (((K * 2) + d) - K);
  var k1: dynamic = (t / now);
  var rem: dynamic = (t % now);
  var ans: dynamic = (d * k1);
  var ans2: dynamic = 0;
  if ((rem <= (K * 2)))
  {
    ans += (rem / 2);
    ans2 = (rem % 2);
  } else
  {
    ans += ((K + rem) - (K * 2));
  }
  printf("%I64d", ans);
  if (ans2)
  {
    write(".5", "\n");
  } else
  {
    write("\n");
  }
  return 0;
}
