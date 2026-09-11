// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var N: dynamic = (1e5 + 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

func main() -> dynamic
{
  f[1] = 1;
  f[2] = 2;
  {
    var i: dynamic = 3;
    while ((i < N))
    {
      f[i] = (((f[(i - 1)] + f[(i - 2)])) % mod);
      i += 1;
    }
  }
  scanf("%d%d", (&n), (&m));
  printf("%d\n", ((2 * (((f[n] + f[m]) - 1))) % mod));
  return 0;
}
