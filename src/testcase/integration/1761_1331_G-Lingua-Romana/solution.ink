// Translated from solution.cpp.

var rnd = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

var N = 11;

var a = cpp_array(N);

func run()
{
  {
    var i = (0);
    while ((i < (N)))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = (N - 1);
    while ((i >= 0))
    {
      var res = (sqrt(abs(a[i])) + (5 * pow(a[i], 3)));
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

func main()
{
  run();
  return 0;
}
