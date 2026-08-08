// Translated from solution.cpp.

var INTERVAL = cpp_expression("#includ");

var EPS = cpp_expression("#inc");

var W: dynamic;

var H: dynamic;

var N: dynamic;

var yama = cpp_array(3, 101);

var xint = cpp_array(2, 101);

func quad(x: dynamic, nyama: dynamic)
{
  var a = yama[nyama][0];
  var p = yama[nyama][1];
  var q = yama[nyama][2];
  return (((a * ((x - p))) * ((x - p))) + q);
}

func x_int(nyama: dynamic, left: dynamic)
{
  var a = yama[nyama][0];
  var p = yama[nyama][1];
  var q = yama[nyama][2];
  return (p + ((1.0 * left) * sqrt(((-q) / a))));
}

func q_int(nyama1: dynamic, nyama2: dynamic, left: dynamic)
{
  var a1 = yama[nyama1][0];
  var p1 = yama[nyama1][1];
  var q1 = yama[nyama1][2];
  var a2 = yama[nyama2][0];
  var p2 = yama[nyama2][1];
  var q2 = yama[nyama2][2];
  if ((a1 == a2))
  {
    if ((p1 == p2))
    {
      return -1.0;
    } else
    {
      var ans = (0.5 * (((p1 + p2) + ((((q2 - q1)) * 1.0) / ((a1 * ((p2 - p1))))))));
      if ((quad(ans, nyama1) > (0 - EPS)))
      {
        return ans;
      } else
      {
        return -1.0;
      }
    }
  } else
  {
    var A = (a1 - a2);
    var B = (2 * (((p2 * a2) - (p1 * a1))));
    var C = (((((a1 * p1) * p1) - ((a2 * p2) * p2)) + q1) - q2);
    var delta = ((B * B) - ((4 * A) * C));
    if ((delta > (0 - EPS)))
    {
      var ans = ((((-B) + ((1.0 * left) * sqrt(delta)))) / ((2.0 * A)));
      if ((quad(ans, nyama1) > (0 - EPS)))
      {
        return ans;
      } else
      {
        return -1.0;
      }
    } else
    {
      return -1.0;
    }
  }
}

func dquad_dx(x: dynamic, nyama: dynamic)
{
  var a = yama[nyama][0];
  var p = yama[nyama][1];
  return ((2.0 * a) * ((x - p)));
}

func cir(x: dynamic, nyama: dynamic)
{
  var fpx = dquad_dx(x, nyama);
  return sqrt((1 + (fpx * fpx)));
}

func integrated(x: dynamic, nyama: dynamic)
{
  var a = yama[nyama][0];
  var fpx = dquad_dx(x, nyama);
  return (((((fpx * cir(x, nyama)) + log(abs((fpx + cir(x, nyama)))))) / 4.0) / a);
}

func integrate(b: dynamic, e: dynamic, nyama: dynamic)
{
  return (integrated(e, nyama) - integrated(b, nyama));
}

var sec: dynamic;

func main()
{
  scanf("%d%d%d", (&W), (&H), (&N));
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%d%d%d", (&yama[i][0]), (&yama[i][1]), (&yama[i][2]));
      xint[i][0] = x_int(i, -1);
      xint[i][1] = x_int(i, 1);
      var tmp1 = x_int(i, -1);
      var tmp2 = x_int(i, 1);
      if ((((-EPS) < tmp1) && (tmp1 < (W + EPS))))
      {
        sec.push_back(tmp1);
      }
      if (((((-EPS) < tmp2) && (tmp2 < (W + EPS))) && (fabs((tmp1 - tmp2)) > EPS)))
      {
        sec.push_back(tmp2);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = (i + 1);
        while ((j < N))
        {
          var tmp1 = q_int(i, j, -1);
          var tmp2 = q_int(i, j, 1);
          if ((((-EPS) < tmp1) && (tmp1 < (W + EPS))))
          {
            sec.push_back(tmp1);
          }
          if (((((-EPS) < tmp2) && (tmp2 < (W + EPS))) && (fabs((tmp1 - tmp2)) > EPS)))
          {
            sec.push_back(tmp2);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(sec.begin(), sec.end());
  sec.erase(unique(sec.begin(), sec.end()), sec.end());
  var ans = 0;
  var mae = 0.0;
  var it = sec.begin();
  while (1)
  {
    var nyama = -1;
    var max = 0.0;
    var now_p: dynamic;
    if ((it == sec.end()))
    {
      now_p = (1.0 * W);
    } else
    {
      now_p = ((*it));
    }
    {
      var j = 0;
      while ((j < N))
      {
        var x = (((mae + now_p)) / 2.0);
        if (((x < (xint[j][0] - EPS)) || (x > (xint[j][1] + EPS))))
        {
          j += 1;
          continue;
        }
        var y = quad(x, j);
        if ((y > (max + EPS)))
        {
          max = y;
          nyama = j;
        }
        j += 1;
      }
    }
    if ((nyama != -1))
    {
      ans += integrate(mae, now_p, nyama);
    }
    if ((it == sec.end()))
    {
      break;
    }
    mae = now_p;
    it += 1;
  }
  printf("%lf\n", ans);
}
