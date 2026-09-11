// Translated from solution.cpp.

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var r: dynamic = cpp_array(20000001);

var n: dynamic = cpp_uninitialized();

func dp(a: dynamic) -> dynamic
{
  if (r[a])
  {
    return r[a];
  }
  if ((a == 1))
  {
    r[a] = x;
  } else if ((a % 2))
  {
    r[a] = (x + min(dp((a - 1)), dp((a + 1))));
  } else
  {
    r[a] = min((y + dp((a / 2))), ((x * ((a / 2))) + dp((a / 2))));
  }
  return r[a];
}

func main() -> dynamic
{
  scanf("%d%I64d%I64d", (&n), (&x), (&y));
  printf("%I64d\n", dp(n));
  return 0;
}
