// Translated from solution.cpp.

var n: dynamic;

func ksm(x: dynamic)
{
  var tt = 1378;
  var rtn = 1;
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

func main()
{
  scanf("%d", (&n));
  printf("%d\n", (ksm(n) % 10));
  return 0;
}
