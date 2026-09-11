// Translated from solution.cpp.

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var xx: dynamic = cpp_array(105);

var yy: dynamic = cpp_array(105);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d%d", (&x), (&y));
      xx[x] = 1;
      yy[y] = 1;
      i += 1;
    }
  }
  var n1: dynamic = 0;
  var n2: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= 100))
    {
      if ((xx[i] == 1))
      {
        n1 += 1;
      }
      if ((yy[i] == 1))
      {
        n2 += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", min(n1, n2));
  return 0;
}
