// Translated from solution.cpp.

var INF = cpp_expression("#include<b");

var ll = dynamic;

var PII = cpp_expression("#include<bits");

func All(a: dynamic)
{
  return cpp_expression("#include<bits/std");
}

var mx = (2e5 + 5);

var mxn = (((1 << 17)) + 5);

var n: dynamic;

var k: dynamic;

var pos = cpp_array(mx, 17);

var dp = cpp_array(mxn);

var s = cpp_array(mx);

func check(mid: dynamic)
{
  {
    var i = 0;
    while ((i < k))
    {
      var cnt = 0;
      {
        var j = n;
        while ((j >= 1))
        {
          cnt = if ((((s[j] == (cpp_char("a") + i)) || (s[j] == cpp_char("?"))))) (cnt + 1) else 0;
          if ((cnt >= mid))
          {
            pos[i][j] = ((j + mid) - 1);
          } else
          {
            pos[i][j] = pos[i][(j + 1)];
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  dp[0] = 0;
  {
    var i = 1;
    while ((i < ((1 << k))))
    {
      dp[i] = INF;
      {
        var j = 0;
        while ((j < k))
        {
          if ((((((i >> j)) & 1) && (dp[(i - ((1 << j)))] != INF)) && pos[j][(dp[(i - ((1 << j)))] + 1)]))
          {
            dp[i] = min(dp[i], pos[j][(dp[(i - ((1 << j)))] + 1)]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return (dp[(((1 << k)) - 1)] != INF);
}

func main()
{
  scanf("%d%d%s", (&n), (&k), (s + 1));
  var l = 1;
  var r = (n / k);
  var res = 0;
  while ((l <= r))
  {
    var mid = (((l + r)) >> 1);
    if (check(mid))
    {
      res = mid;
      l = (mid + 1);
    } else
    {
      r = (mid - 1);
    }
  }
  printf("%d", res);
}
