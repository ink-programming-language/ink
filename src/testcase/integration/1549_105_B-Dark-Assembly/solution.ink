// Translated from solution.cpp.

var PI: dynamic = acos(-1.0);

var eps: dynamic = 1e-8;

var tot: dynamic = 0;

var cnt: dynamic = cpp_array(8);

var lvl: dynamic = cpp_array(8);

var loy: dynamic = cpp_array(8);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var ans: dynamic = 0;

func gao() -> dynamic
{
  var curWin: dynamic = 0;
  {
    var mask: dynamic = 0;
    while ((mask < ((1 << n))))
    {
      var winnum: dynamic = 0;
      var B: dynamic = 0;
      var pers: dynamic = 1;
      {
        var i: dynamic = 0;
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

func dfs(p: dynamic, left: dynamic) -> dynamic
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
      var i: dynamic = 0;
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

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  scanf("%d%d%d", (&n), (&k), (&A));
  {
    var i: dynamic = 0;
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
