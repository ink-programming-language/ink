// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%lld%lld%lld%lld%lld", (&n), (&a), (&b), (&c), (&d));
  n -= 1;
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      if (((((c * i) - (d * ((n - i)))) <= (a - b)) && ((a - b) <= ((d * i) - (c * ((n - i)))))))
      {
        e = 1;
      }
      i += 1;
    }
  }
  if (e)
  {
    printf("YES\n");
  } else
  {
    printf("NO\n");
  }
}
