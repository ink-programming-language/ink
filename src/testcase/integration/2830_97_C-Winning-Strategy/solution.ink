// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(110);

var mval: dynamic = -1.0;

func iabs(x: dynamic) -> dynamic
{
  return  ((x < 0)) ? (-x) : x;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      scanf("%lf", (&p[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      var a: dynamic = (n - (2 * i));
      if ((a <= 0))
      {
        break;
      }
      {
        var j: dynamic = n;
        while ((j >= 0))
        {
          var b: dynamic = (n - (2 * j));
          if ((b >= 0))
          {
            break;
          }
          if (((!a) && (!b)))
          {
            j -= 1;
            continue;
          }
          var val: dynamic = ((((p[i] * cpp_double(iabs(b))) + (p[j] * cpp_double(iabs(a))))) / cpp_double((iabs(a) + iabs(b))));
          mval = max(mval, val);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  if (((n % 2) == 0))
  {
    mval = max(mval, p[(n / 2)]);
  }
  printf("%.9lf", mval);
  return 0;
}
