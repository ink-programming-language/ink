// Translated from solution.cpp.

var n: dynamic;

var f = cpp_array(400);

func main()
{
  scanf("%d", (&n));
  f[0] = 2;
  {
    var i = 0;
    while ((i <= n))
    {
      f[i] = ((f[(i - 1)] * 2) + 2);
      i += 1;
    }
  }
  printf("%lld\n", f[n]);
  return 0;
}
