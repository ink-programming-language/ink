// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

func Get(x: dynamic) -> dynamic
{
  if (mp.count(x))
  {
    return mp[x];
  }
  printf("? %d\n", x);
  fflush(stdout);
  var y: dynamic = cpp_uninitialized();
  scanf("%d", (&y));
  return cpp_assign(mp[x], "=", y);
}

func Print(x: dynamic) -> dynamic
{
  printf("! %d\n", x);
  exit(0);
}

func main() -> dynamic
{
  scanf("%d", (&n));
  if ((n % 4))
  {
    Print(-1);
  }
  var a: dynamic = Get((n / 2));
  var b: dynamic = Get(n);
  var l1: dynamic = 1;
  var r1: dynamic = ((n / 2) - 1);
  var l2: dynamic = ((n / 2) + 1);
  var r2: dynamic = (n - 1);
  if ((a == b))
  {
    Print(n);
  }
  while (true)
  {
    var mid1: dynamic = (((l1 + r1)) >> 1);
    var mid2: dynamic = (((l2 + r2)) >> 1);
    var x: dynamic = Get(mid1);
    var y: dynamic = Get(mid2);
    if ((x == y))
    {
      Print(mid1);
    } else if (((((a < b) && (x > y))) || (((a > b) && (x < y)))))
    {
      l1 = (mid1 + 1);
      l2 = (mid2 + 1);
    } else
    {
      r1 = (mid1 - 1);
      r2 = (mid2 - 1);
    }
  }
  return 0;
}
