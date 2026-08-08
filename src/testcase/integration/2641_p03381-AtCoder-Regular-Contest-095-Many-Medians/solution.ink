// Translated from solution.cpp.

var N = (200000 + 5);

func main()
{
  var n: dynamic;
  var a = cpp_array(N);
  var b = cpp_array(N);
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      b[i] = a[i];
      i += 1;
    }
  }
  sort(b, (b + n));
  var x: dynamic;
  var y: dynamic;
  y = b[(n / 2)];
  x = b[((n / 2) - 1)];
  {
    var i = 0;
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
