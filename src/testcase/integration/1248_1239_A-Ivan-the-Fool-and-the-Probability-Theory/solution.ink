// Translated from solution.cpp.

var mod = (1e9 + 7);

var N = (1e5 + 10);

var n: dynamic;

var m: dynamic;

var f = cpp_array(N);

func main()
{
  f[1] = 1;
  f[2] = 2;
  {
    var i = 3;
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
