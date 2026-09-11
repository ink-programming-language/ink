// Translated from solution.cpp.

var ara: dynamic = cpp_array(100005);

var aa: dynamic = cpp_array(100005);

var bb: dynamic = cpp_array(100005);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (ara + i));
      i += 1;
    }
  }
  if ((n == 1))
  {
    printf("1 1\n%lld\n", (-ara[1]));
    printf("1 1\n0\n");
    printf("1 1\n0\n");
    return 0;
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var x: dynamic = ara[i];
      var rem: dynamic = (((-x)) % n);
      rem += n;
      rem %= n;
      rem = (((n - rem)) % n);
      var b: dynamic = rem;
      var a: dynamic = ((((b - x)) / n) - b);
      aa[i] = (a * n);
      bb[i] = (b * ((n - 1)));
      i += 1;
    }
  }
  printf("%d %d\n", 1, n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%lld ", aa[i]);
      i += 1;
    }
  }
  printf("\n");
  printf("%d %d\n", 1, (n - 1));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      printf("%lld ", bb[i]);
      i += 1;
    }
  }
  printf("\n");
  printf("%d %d\n", n, n);
  printf("%lld\n", (-ara[n]));
  return 0;
}
