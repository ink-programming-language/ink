// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(400);

func main() -> dynamic
{
  scanf("%d", (&n));
  f[0] = 2;
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      f[i] = ((f[(i - 1)] * 2) + 2);
      i += 1;
    }
  }
  printf("%lld\n", f[n]);
  return 0;
}
