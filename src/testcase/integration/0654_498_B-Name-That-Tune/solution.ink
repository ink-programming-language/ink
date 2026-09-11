// Translated from solution.cpp.

var MAX: dynamic = 5010;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&T));
  var ds1: dynamic = [];
  var ds2: dynamic = [];
  var rs: dynamic = ds1;
  var rs2: dynamic = ds2;
  var res: dynamic = [];
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      rs2[0] = (i == 0);
      var P: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf("%d%d", (&P), (&t));
      var sm: dynamic = 0;
      var p: dynamic = (P / 100.0);
      var q: dynamic = (1 - p);
      var qt1: dynamic = pow(q, (t - 1));
      {
        var j: dynamic = max((i - 1), 1);
        while ((j < t))
        {
          if ((sm != 0))
          {
            sm *= q;
          }
          sm += (rs2[(j - 1)] * p);
          rs[j] = sm;
          j += 1;
        }
      }
      {
        var j: dynamic = max((i - 1), t);
        while ((j <= T))
        {
          if ((sm != 0))
          {
            sm *= q;
          }
          sm += (rs2[(j - 1)] * p);
          if ((rs2[(j - t)] != 0))
          {
            var v: dynamic = (rs2[(j - t)] * qt1);
            sm -= ((rs2[(j - t)] * qt1) * p);
            rs[j] = (sm + v);
          } else
          {
            rs[j] = sm;
          }
          j += 1;
        }
      }
      var Q: dynamic = 1;
      {
        var j: dynamic = T;
        while (((j >= ((T - t) + 1)) && (j >= i)))
        {
          res[i] += (rs2[j] * Q);
          Q *= q;
          j -= 1;
        }
      }
      swap(rs, rs2);
      i += 1;
    }
  }
  {
    var j: dynamic = 1;
    while ((j <= T))
    {
      res[n] += rs2[j];
      j += 1;
    }
  }
  var r: dynamic = 0;
  {
    var j: dynamic = 1;
    while ((j <= T))
    {
      r += (res[j] * j);
      j += 1;
    }
  }
  printf("%.10lf\n", r);
  return 0;
}
