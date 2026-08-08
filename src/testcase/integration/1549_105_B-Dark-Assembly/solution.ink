// Translated from solution.cpp.

var PI = acos(-1.0);

var eps = 1e-8;

var tot = 0;

var cnt = cpp_array(8);

var lvl = cpp_array(8);

var loy = cpp_array(8);

var n: dynamic;

var k: dynamic;

var A: dynamic;

var ans = 0;

func gao()
{
  var curWin = 0;
  {
    var mask = 0;
    while ((mask < ((1 << n))))
    {
      var winnum = 0;
      var B = 0;
      var pers = 1;
      {
        var i = 0;
        while ((i < n))
        {
          if ((mask & ((1 << i))))
          {
            winnum += 1;
            pers *= ((1.0 * loy[i]) / 100);
          } else
          {
            B += lvl[i];
            pers *= (1 - ((1.0 * loy[i]) / 100));
          }
          i += 1;
        }
      }
      if ((winnum <= (n - winnum)))
      {
        curWin += (((1.0 * pers) * A) / ((A + B)));
      } else
      {
        curWin += pers;
      }
      mask += 1;
    }
  }
  if ((curWin > ans))
  {
    ans = curWin;
  }
}

func dfs(p: dynamic, left: dynamic)
{
  if ((p == (n - 1)))
  {
    left = min(left, (((100 - loy[p])) / 10));
    loy[p] += (left * 10);
    gao();
    loy[p] -= (left * 10);
  } else
  {
    {
      var i = 0;
      while (((i <= left) && ((loy[p] + (i * 10)) <= 100)))
      {
        loy[p] += (i * 10);
        dfs((p + 1), (left - i));
        loy[p] -= (i * 10);
        i += 1;
      }
    }
  }
}

func main(argc: dynamic, argv: dynamic)
{
  scanf("%d%d%d", (&n), (&k), (&A));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d%d", (&lvl[i]), (&loy[i]));
      i += 1;
    }
  }
  dfs(0, k);
  printf("%.6lf\n", ans);
  return 0;
}
