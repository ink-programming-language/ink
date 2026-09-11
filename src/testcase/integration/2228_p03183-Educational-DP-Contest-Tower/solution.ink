// Translated from solution.cpp.

var s: dynamic = cpp_expression("#incl");

var w: dynamic = cpp_expression("#include<bit");

var v: dynamic = cpp_expression("#include<bits");

var maxn: dynamic = cpp_expression("#inc");

var maxs: dynamic = cpp_expression("#incl");

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var b: dynamic = cpp_array(maxn);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d%d%d", (&b[i].w), (&b[i].s), (&b[i].v));
      b[i].s += b[i].w;
      i += 1;
    }
  }
  sort(b, (b + n));
  var dp: dynamic = [];
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = b[i].s;
        while ((j >= b[i].w))
        {
          dp[j] = max(dp[j], (dp[(j - b[i].w)] + b[i].v));
          ans = max(ans, dp[j]);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
