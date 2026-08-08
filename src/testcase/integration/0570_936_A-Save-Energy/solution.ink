// Translated from solution.cpp.

var K: dynamic;

var d: dynamic;

var t: dynamic;

func main()
{
  scanf("%I64d%I64d%I64d", (&K), (&d), (&t));
  d = ((((((K - 1)) / d) + 1)) * d);
  t = (t * 2);
  var now = (((K * 2) + d) - K);
  var k1 = (t / now);
  var rem = (t % now);
  var ans = (d * k1);
  var ans2 = 0;
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
