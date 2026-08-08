// Translated from solution.cpp.

var x: dynamic;

var y: dynamic;

var r = cpp_array(20000001);

var n: dynamic;

func dp(a: dynamic)
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

func main()
{
  scanf("%d%I64d%I64d", (&n), (&x), (&y));
  printf("%I64d\n", dp(n));
  return 0;
}
