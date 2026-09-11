// Translated from solution.cpp.

var rnd: dynamic = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

var N: dynamic = 11;

var a: dynamic = cpp_array(N);

func run() -> dynamic
{
  {
    var i: dynamic = (0);
    while ((i < (N)))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = (N - 1);
    while ((i >= 0))
    {
      var res: dynamic = (sqrt(abs(a[i])) + (5 * pow(a[i], 3)));
      if ((res <= 400))
      {
        printf("f(%d) = %.2lf\n", a[i], res);
      } else
      {
        printf("f(%d) = MAGNA NIMIS!\n", a[i]);
      }
      i -= 1;
    }
  }
}

func main() -> dynamic
{
  run();
  return 0;
}
