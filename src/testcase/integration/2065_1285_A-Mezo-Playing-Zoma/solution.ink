// Translated from solution.cpp.

var N: dynamic = (1e6 + 10);

var inf: dynamic = 0x3f3f3f3f;

var inf2: dynamic = (1e18 + 10);

var mod: dynamic = 1000000007;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var str: dynamic = cpp_array(100100);
  scanf("%d", (&n));
  scanf("%*s", str);
  printf("%d\n", (n + 1));
  return 0;
}
