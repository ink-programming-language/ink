// Translated from solution.cpp.

func main()
{
  var y1: dynamic;
  var m1: dynamic;
  var d1: dynamic;
  var y2: dynamic;
  var m2: dynamic;
  var d2: dynamic;
  while ((((((((cin >> y1) >> m1) >> d1) >> y2) >> m2) >> d2) && (y1 != -1)))
  {
    var res = 0;
    var days = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    {
      var y = y1;
      while ((y <= y2))
      {
        var ms = 1;
        var me = 12;
        if ((y == y1))
        {
          ms = m1;
        }
        if ((y == y2))
        {
          me = m2;
        }
        {
          var m = ms;
          while ((m <= me))
          {
            var ds = 1;
            var de = days[m];
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
              var d = ds;
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
