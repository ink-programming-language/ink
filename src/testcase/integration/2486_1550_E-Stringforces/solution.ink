// Translated from solution.cpp.

var INF: dynamic = cpp_expression("#include<b");

var ll: dynamic = dynamic;

var PII: dynamic = cpp_expression("#include<bits");

func All(a: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/std");
}

var mx: dynamic = (2e5 + 5);

var mxn: dynamic = (((1 << 17)) + 5);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var pos: dynamic = cpp_array(mx, 17);

var dp: dynamic = cpp_array(mxn);

var s: dynamic = cpp_array(mx);

func check(mid: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      var cnt: dynamic = 0;
      {
        var j: dynamic = n;
        while ((j >= 1))
        {
          cnt =  ((((s[j] == (cpp_char("a") + i)) || (s[j] == cpp_char("?"))))) ? (cnt + 1) : 0;
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
    var i: dynamic = 1;
    while ((i < ((1 << k))))
    {
      dp[i] = INF;
      {
        var j: dynamic = 0;
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

func main() -> dynamic
{
  scanf("%d%d%s", (&n), (&k), (s + 1));
  var l: dynamic = 1;
  var r: dynamic = (n / k);
  var res: dynamic = 0;
  while ((l <= r))
  {
    var mid: dynamic = (((l + r)) >> 1);
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
