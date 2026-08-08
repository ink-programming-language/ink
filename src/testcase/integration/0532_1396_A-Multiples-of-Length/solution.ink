// Translated from solution.cpp.

var ara = cpp_array(100005);

var aa = cpp_array(100005);

var bb = cpp_array(100005);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
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
    var i = 1;
    while ((i < n))
    {
      var x = ara[i];
      var rem = (((-x)) % n);
      rem += n;
      rem %= n;
      rem = (((n - rem)) % n);
      var b = rem;
      var a = ((((b - x)) / n) - b);
      aa[i] = (a * n);
      bb[i] = (b * ((n - 1)));
      i += 1;
    }
  }
  printf("%d %d\n", 1, n);
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%lld ", aa[i]);
      i += 1;
    }
  }
  printf("\n");
  printf("%d %d\n", 1, (n - 1));
  {
    var i = 1;
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
