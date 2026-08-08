// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(201);

var b = cpp_array(201);

var c = cpp_array(201);

func main()
{
  scanf("%d", (&n));
  var sum = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&b[i]));
      sum += b[i];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    var i = 1;
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
