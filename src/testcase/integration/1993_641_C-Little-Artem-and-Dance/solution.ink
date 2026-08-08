// Translated from solution.cpp.

var b = cpp_array(1100000);

func main()
{
  var n: dynamic;
  var q: dynamic;
  var odd: dynamic;
  var even: dynamic;
  var p1: dynamic;
  var p2: dynamic;
  while ((scanf("%d %d", (&n), (&q)) != EOF))
  {
    memset(b, 0, cpp_sizeof((b)));
    odd = 0;
    even = 1;
    while (cpp_update(q, "--"))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d", (&x));
      if ((x == 1))
      {
        scanf("%d", (&y));
        odd = ((((odd + y) + n)) % n);
        even = ((((even + y) + n)) % n);
      } else
      {
        if ((odd % 2))
        {
          odd = ((((odd - 1) + n)) % n);
          even = ((((even + 1) + n)) % n);
        } else
        {
          odd = ((((odd + 1) + n)) % n);
          even = ((((even - 1) + n)) % n);
        }
      }
    }
    var cnt = 1;
    while ((cnt <= n))
    {
      b[odd] = cpp_update(cnt, "++");
      b[even] = cpp_update(cnt, "++");
      odd = ((((odd + 2) + n)) % n);
      even = ((((even + 2) + n)) % n);
    }
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        printf("%d ", b[i]);
        i += 1;
      }
    }
    printf("%d\n", b[(n - 1)]);
  }
  return 0;
}
