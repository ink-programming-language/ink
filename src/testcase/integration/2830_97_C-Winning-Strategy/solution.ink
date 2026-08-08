// Translated from solution.cpp.

var n: dynamic;

var p = cpp_array(110);

var mval = -1.0;

func iabs(x: dynamic)
{
  return if ((x < 0)) (-x) else x;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i <= n))
    {
      scanf("%lf", (&p[i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      var a = (n - (2 * i));
      if ((a <= 0))
      {
        break;
      }
      {
        var j = n;
        while ((j >= 0))
        {
          var b = (n - (2 * j));
          if ((b >= 0))
          {
            break;
          }
          if (((!a) && (!b)))
          {
            j -= 1;
            continue;
          }
          var val = ((((p[i] * double(iabs(b))) + (p[j] * double(iabs(a))))) / double((iabs(a) + iabs(b))));
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
