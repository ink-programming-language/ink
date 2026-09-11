// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(201);

var b: dynamic = cpp_array(201);

var c: dynamic = cpp_array(201);

func main() -> dynamic
{
  scanf("%d", (&n));
  var sum: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i]));
      sum += b[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      if (((a[i] + 1) == a[(i + 1)]))
      {
        sum += c[a[i]];
      }
      i += 1;
    }
  }
  printf("%d\n", sum);
  return 0;
}
