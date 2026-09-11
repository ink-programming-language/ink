// Translated from solution.cpp.

var INTERVAL: dynamic = cpp_expression("#includ");

var EPS: dynamic = cpp_expression("#inc");

var W: dynamic = cpp_uninitialized();

var H: dynamic = cpp_uninitialized();

var N: dynamic = cpp_uninitialized();

var yama: dynamic = cpp_array(3, 101);

var xint: dynamic = cpp_array(2, 101);

func quad(x: dynamic, nyama: dynamic) -> dynamic
{
  var a: dynamic = yama[nyama][0];
  var p: dynamic = yama[nyama][1];
  var q: dynamic = yama[nyama][2];
  return (((a * ((x - p))) * ((x - p))) + q);
}

func x_int(nyama: dynamic, left: dynamic) -> dynamic
{
  var a: dynamic = yama[nyama][0];
  var p: dynamic = yama[nyama][1];
  var q: dynamic = yama[nyama][2];
  return (p + ((1.0 * left) * sqrt(((-q) / a))));
}

func q_int(nyama1: dynamic, nyama2: dynamic, left: dynamic) -> dynamic
{
  var a1: dynamic = yama[nyama1][0];
  var p1: dynamic = yama[nyama1][1];
  var q1: dynamic = yama[nyama1][2];
  var a2: dynamic = yama[nyama2][0];
  var p2: dynamic = yama[nyama2][1];
  var q2: dynamic = yama[nyama2][2];
  if ((a1 == a2))
  {
    if ((p1 == p2))
    {
      return -1.0;
    } else
    {
      var ans: dynamic = (0.5 * (((p1 + p2) + ((((q2 - q1)) * 1.0) / ((a1 * ((p2 - p1))))))));
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
    var A: dynamic = (a1 - a2);
    var B: dynamic = (2 * (((p2 * a2) - (p1 * a1))));
    var C: dynamic = (((((a1 * p1) * p1) - ((a2 * p2) * p2)) + q1) - q2);
    var delta: dynamic = ((B * B) - ((4 * A) * C));
    if ((delta > (0 - EPS)))
    {
      var ans: dynamic = ((((-B) + ((1.0 * left) * sqrt(delta)))) / ((2.0 * A)));
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

func dquad_dx(x: dynamic, nyama: dynamic) -> dynamic
{
  var a: dynamic = yama[nyama][0];
  var p: dynamic = yama[nyama][1];
  return ((2.0 * a) * ((x - p)));
}

func cir(x: dynamic, nyama: dynamic) -> dynamic
{
  var fpx: dynamic = dquad_dx(x, nyama);
  return sqrt((1 + (fpx * fpx)));
}

func integrated(x: dynamic, nyama: dynamic) -> dynamic
{
  var a: dynamic = yama[nyama][0];
  var fpx: dynamic = dquad_dx(x, nyama);
  return (((((fpx * cir(x, nyama)) + log(abs((fpx + cir(x, nyama)))))) / 4.0) / a);
}

func integrate(b: dynamic, e: dynamic, nyama: dynamic) -> dynamic
{
  return (integrated(e, nyama) - integrated(b, nyama));
}

var sec: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%d%d", (&W), (&H), (&N));
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      scanf("%d%d%d", (&yama[i][0]), (&yama[i][1]), (&yama[i][2]));
      xint[i][0] = x_int(i, -1);
      xint[i][1] = x_int(i, 1);
      var tmp1: dynamic = x_int(i, -1);
      var tmp2: dynamic = x_int(i, 1);
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
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < N))
        {
          var tmp1: dynamic = q_int(i, j, -1);
          var tmp2: dynamic = q_int(i, j, 1);
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
  var ans: dynamic = 0;
  var mae: dynamic = 0.0;
  var it: dynamic = sec.begin();
  while (1)
  {
    var nyama: dynamic = -1;
    var max: dynamic = 0.0;
    var now_p: dynamic = cpp_uninitialized();
    if ((it == sec.end()))
    {
      now_p = (1.0 * W);
    } else
    {
      now_p = ((*it));
    }
    {
      var j: dynamic = 0;
      while ((j < N))
      {
        var x: dynamic = (((mae + now_p)) / 2.0);
        if (((x < (xint[j][0] - EPS)) || (x > (xint[j][1] + EPS))))
        {
          j += 1;
          continue;
        }
        var y: dynamic = quad(x, j);
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
