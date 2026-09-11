// Translated from solution.cpp.

func main() -> dynamic
{
  var y1: dynamic = cpp_uninitialized();
  var m1: dynamic = cpp_uninitialized();
  var d1: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  var m2: dynamic = cpp_uninitialized();
  var d2: dynamic = cpp_uninitialized();
  while ((((((((cin >> y1) >> m1) >> d1) >> y2) >> m2) >> d2) && (y1 != -1)))
  {
    var res: dynamic = 0;
    var days: dynamic = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    {
      var y: dynamic = y1;
      while ((y <= y2))
      {
        var ms: dynamic = 1;
        var me: dynamic = 12;
        if ((y == y1))
        {
          ms = m1;
        }
        if ((y == y2))
        {
          me = m2;
        }
        {
          var m: dynamic = ms;
          while ((m <= me))
          {
            var ds: dynamic = 1;
            var de: dynamic = days[m];
            if (((m == 2) && ((y % 4) == 0)))
            {
              if ((((y % 100) != 0) || ((y % 400) == 0)))
              {
                de += 1;
              }
            }
            if (((y == y1) && (m == m1)))
            {
              ds = d1;
            }
            if (((y == y2) && (m == m2)))
            {
              de = d2;
            }
            {
              var d: dynamic = ds;
              while ((d <= de))
              {
                res += 1;
                d += 1;
              }
            }
            m += 1;
          }
        }
        y += 1;
      }
    }
    write((res - 1), "\n");
  }
  return 0;
}
