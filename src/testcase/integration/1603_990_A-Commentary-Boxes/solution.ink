// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var ans1: dynamic = cpp_uninitialized();
  var ans2: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  scanf("%I64d%I64d%I64d%I64d", (&n), (&m), (&a), (&b));
  if (((n % m) == 0))
  {
    printf("");
  }
  if ((n < m))
  {
    ans1 = ((((m - n)) * a));
    ans2 = (n * b);
  } else
  {
    ans1 = (((n % m)) * b);
    ans2 = (((((((((n / m)) + 1)) * m)) - n)) * a);
  }
  if ((ans1 < ans2))
  {
    printf("%I64d", ans1);
  } else
  {
    printf("%I64d", ans2);
  }
  return 0;
}
