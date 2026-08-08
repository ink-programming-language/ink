// Translated from solution.cpp.

var MAX = 5010;

func main()
{
  var n: dynamic;
  var T: dynamic;
  scanf("%d%d", (&n), (&T));
  var ds1 = [];
  var ds2 = [];
  var rs = ds1;
  var rs2 = ds2;
  var res = [];
  {
    var i = 0;
    while ((i < n))
    {
      rs2[0] = (i == 0);
      var P: dynamic;
      var t: dynamic;
      scanf("%d%d", (&P), (&t));
      var sm = 0;
      var p = (P / 100.0);
      var q = (1 - p);
      var qt1 = pow(q, (t - 1));
      {
        var j = max((i - 1), 1);
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
        var j = max((i - 1), t);
        while ((j <= T))
        {
          if ((sm != 0))
          {
            sm *= q;
          }
          sm += (rs2[(j - 1)] * p);
          if ((rs2[(j - t)] != 0))
          {
            var v = (rs2[(j - t)] * qt1);
            sm -= ((rs2[(j - t)] * qt1) * p);
            rs[j] = (sm + v);
          } else
          {
            rs[j] = sm;
          }
          j += 1;
        }
      }
      var Q = 1;
      {
        var j = T;
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
    var j = 1;
    while ((j <= T))
    {
      res[n] += rs2[j];
      j += 1;
    }
  }
  var r = 0;
  {
    var j = 1;
    while ((j <= T))
    {
      r += (res[j] * j);
      j += 1;
    }
  }
  printf("%.10lf\n", r);
  return 0;
}
