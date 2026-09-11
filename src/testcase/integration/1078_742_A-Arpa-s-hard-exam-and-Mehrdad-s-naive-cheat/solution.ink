// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

func ksm(x: dynamic) -> dynamic
{
  var tt: dynamic = 1378;
  var rtn: dynamic = 1;
  while (x)
  {
    if ((x & 1))
    {
      rtn = (((rtn * tt)) % 10);
    }
    x >>= 1;
    tt = (((tt * tt)) % 10);
  }
  return rtn;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  printf("%d\n", (ksm(n) % 10));
  return 0;
}
