// Translated from solution.cpp.

var EPS = 1e-8;

func rot(a: dynamic, b: dynamic)
{
  return arg((conj(a) * b));
}

func operator_less(a: dynamic, b: dynamic)
{
  return (make_pair(real(a), imag(a)) < make_pair(real(b), imag(b)));
}

func main()
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    var g1 = P(0, 0);
    var g2 = P(0, 0);
    var x: dynamic;
    var y: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        read(x, y);
        vp1[i] = P(x, y);
        g1 += vp1[i];
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        read(x, y);
        vp2[i] = P(x, y);
        g2 += vp2[i];
        i += 1;
      }
    }
    g1 /= n;
    g2 /= n;
    {
      var i = 0;
      while ((i < n))
      {
        vp1[i] -= g1;
        vp2[i] -= g2;
        i += 1;
      }
    }
    sort(vp1.begin(), vp1.end());
    var ans = 1e12;
    {
      var i = 0;
      while ((i < n))
      {
        if ((abs((abs(vp1[0]) - abs(vp2[i]))) < EPS))
        {
          var r = rot(vp2[i], vp1[0]);
          {
            var j = 0;
            while ((j < n))
            {
              tmp[j] = P(((real(vp2[j]) * cos(r)) - (imag(vp2[j]) * sin(r))), ((real(vp2[j]) * sin(r)) + (imag(vp2[j]) * cos(r))));
              j += 1;
            }
          }
          sort(tmp.begin(), tmp.end());
          var flag = true;
          {
            var j = 0;
            while ((j < n))
            {
              flag &= ((abs((vp1[j] - tmp[j])) < EPS));
              j += 1;
            }
          }
          if (flag)
          {
            ans = min(ans, abs(r));
          }
        }
        i += 1;
      }
    }
    printf("%.10lf\n", ans);
  }
}
