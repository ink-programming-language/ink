// Translated from solution.cpp.

var N: dynamic;

var a: dynamic;

var m: dynamic;

func main()
{
  scanf("%lld", (&N));
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%lld", (&a));
      m[a] += 1;
      i += 1;
    }
  }
  a = 0;
  for (var i in m)
  {
    a += if ((i.second >= i.first)) (i.second - i.first) else i.second;
  }
  write(a, "\n");
  return 0;
}
