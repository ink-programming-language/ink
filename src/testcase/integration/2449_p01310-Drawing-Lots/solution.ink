// Translated from solution.cpp.

var fr = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

var INF = 1000000000;

class SEG
{
  var siz: dynamic;
  var s: dynamic = cpp_array((1 << 18));
  func init()
  {
      siz = (1 << 17);
      {
        var i = 0;
        while ((i < ((2 * siz) - 1)))
        {
          s[i] = INF;
          i += 1;
        }
      }
    }
  func updata(a: dynamic, b: dynamic, x: dynamic, k: dynamic, l: dynamic, r: dynamic)
  {
      if (((b <= l) || (r <= a)))
      {
        return;
      }
      if (((a <= l) && (r <= b)))
      {
        s[k] = min(s[k], x);
        return;
      }
      updata(a, b, x, ((2 * k) + 1), l, (((l + r)) / 2));
      updata(a, b, x, ((2 * k) + 2), (((l + r)) / 2), r);
    }
  func updata(a: dynamic, x: dynamic, k: dynamic, l: dynamic, r: dynamic)
  {
      if ((((a + 1) <= l) || (r <= a)))
      {
        return;
      }
      if (((a == l) && (r == (a + 1))))
      {
        s[k] = x;
        return;
      }
      updata(l, r, s[k], ((2 * k) + 1), l, (((l + r)) / 2));
      updata(l, r, s[k], ((2 * k) + 2), (((l + r)) / 2), r);
      s[k] = INF;
      updata(a, x, ((2 * k) + 1), l, (((l + r)) / 2));
      updata(a, x, ((2 * k) + 2), (((l + r)) / 2), r);
    }
  func query(a: dynamic)
  {
      a += (siz - 1);
      var ret = s[a];
      while ((a > 0))
      {
        a = (((a - 1)) / 2);
        ret = min(ret, s[a]);
      }
      return ret;
    }
}

var dp = cpp_array(2);

func main()
{
  var T: dynamic;
  scanf("%d", (&T));
  {
    var test = 0;
    while ((test < T))
    {
      var n: dynamic;
      var m: dynamic;
      var k: dynamic;
      var h = cpp_array(100010);
      var x = cpp_array(100010);
      scanf("%d%d%d", (&n), (&m), (&k));
      {
        var i = 0;
        while ((i < m))
        {
          scanf("%d%d", (&h[i]), (&x[i]));
          i += 1;
        }
      }
      var p = cpp_array(100010);
      {
        var i = 0;
        while ((i < m))
        {
          p[i] = pair(h[i], x[i]);
          i += 1;
        }
      }
      sort(p, (p + m));
      dp[0].init();
      dp[1].init();
      dp[0].updata(k, (n + 1), (-k), 0, 0, dp[0].siz);
      dp[1].updata(1, (k + 1), k, 0, 0, dp[1].siz);
      var t = 0;
      var now = k;
      {
        var i = 0;
        while ((i < m))
        {
          var c = [1, -1];
          if ((p[i].sc == now))
          {
            now += 1;
          } else if (((p[i].sc + 1) == now))
          {
            now -= 1;
          }
          {
            var j = 0;
            while ((j < 2))
            {
              var memo = dp[j].query(p[i].sc);
              dp[j].updata(p[i].sc, min((dp[j].query((p[i].sc + 1)) + c[j]), (abs((now - p[i].sc)) - (p[i].sc * c[j]))), 0, 0, dp[j].siz);
              dp[j].updata((p[i].sc + 1), min((memo - c[j]), (abs(((now - p[i].sc) - 1)) - (((p[i].sc + 1)) * c[j]))), 0, 0, dp[j].siz);
              j += 1;
            }
          }
          if ((((i + 1) < m) && (p[i].fr == p[(i + 1)].fr)))
          {
            i += 1;
            continue;
          }
          {
            while ((t <= i))
            {
              dp[0].updata((p[t].sc + 1), (n + 1), min(dp[0].query((p[t].sc + 1)), (dp[1].query((p[t].sc + 1)) - (2 * ((p[t].sc + 1))))), 0, 0, dp[0].siz);
              dp[1].updata(1, (p[t].sc + 1), min((dp[0].query(p[t].sc) + (2 * p[t].sc)), dp[1].query(p[t].sc)), 0, 0, dp[1].siz);
              dp[0].updata((p[t].sc - 1), (n + 1), min(dp[0].query((p[t].sc - 1)), (dp[1].query((p[t].sc - 1)) - (2 * ((p[t].sc - 1))))), 0, 0, dp[0].siz);
              dp[1].updata(1, (p[t].sc + 3), min((dp[0].query((p[t].sc + 2)) + (2 * ((p[t].sc + 2)))), dp[1].query((p[t].sc + 2))), 0, 0, dp[1].siz);
              t += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= n))
        {
          printf("%d%c", min((dp[0].query(i) + i), (dp[1].query(i) - i)), 10);
          i += 1;
        }
      }
      test += 1;
    }
  }
}
