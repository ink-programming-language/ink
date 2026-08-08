// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var k1: dynamic;

var k2: dynamic;

var n: dynamic;

var x: dynamic;

var k = 0;

func main()
{
  scanf("%d", (&n));
  scanf("%d", (&x));
  a.insert(x);
  b[x] = 1;
  {
    var i = 1;
    while ((i < n))
    {
      scanf("%d", (&x));
      k1 = a.lower_bound(x);
      if ((k1 == a.end()))
      {
        k1 -= 1;
        printf("%d ", (*k1));
      } else if ((k1 == a.begin()))
      {
        printf("%d ", (*k1));
      } else
      {
        k2 = k1;
        k1 -= 1;
        if ((b[(*k2)] < b[(*k1)]))
        {
          printf("%d ", (*k1));
        } else
        {
          printf("%d ", (*k2));
        }
      }
      a.insert(x);
      b[x] = (i + 1);
      i += 1;
    }
  }
  return 0;
}
