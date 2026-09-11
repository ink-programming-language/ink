// Translated from solution.cpp.

var N: dynamic = (200000 + 5);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(N);
  var b: dynamic = cpp_array(N);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      b[i] = a[i];
      i += 1;
    }
  }
  sort(b, (b + n));
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  y = b[(n / 2)];
  x = b[((n / 2) - 1)];
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((a[i] <= x))
      {
        printf("%d\n", y);
      } else if ((a[i] >= y))
      {
        printf("%d\n", x);
      }
      i += 1;
    }
  }
  return 0;
}
