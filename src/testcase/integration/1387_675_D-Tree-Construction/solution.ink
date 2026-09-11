// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var k1: dynamic = cpp_uninitialized();

var k2: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var k: dynamic = 0;

func main() -> dynamic
{
  scanf("%d", (&n));
  scanf("%d", (&x));
  a.insert(x);
  b[x] = 1;
  {
    var i: dynamic = 1;
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
