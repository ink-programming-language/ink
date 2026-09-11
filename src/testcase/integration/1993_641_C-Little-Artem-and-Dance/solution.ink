// Translated from solution.cpp.

var b: dynamic = cpp_array(1100000);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var odd: dynamic = cpp_uninitialized();
  var even: dynamic = cpp_uninitialized();
  var p1: dynamic = cpp_uninitialized();
  var p2: dynamic = cpp_uninitialized();
  while ((scanf("%d %d", (&n), (&q)) != EOF))
  {
    memset(b, 0, cpp_sizeof((b)));
    odd = 0;
    even = 1;
    while (cpp_update(q, "--"))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
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
    var cnt: dynamic = 1;
    while ((cnt <= n))
    {
      b[odd] = cpp_update(cnt, "++");
      b[even] = cpp_update(cnt, "++");
      odd = ((((odd + 2) + n)) % n);
      even = ((((even + 2) + n)) % n);
    }
    {
      var i: dynamic = 0;
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
