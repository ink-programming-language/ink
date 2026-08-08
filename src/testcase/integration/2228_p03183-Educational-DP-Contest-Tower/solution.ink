// Translated from solution.cpp.

var s = cpp_expression("#incl");

var w = cpp_expression("#include<bit");

var v = cpp_expression("#include<bits");

var maxn = cpp_expression("#inc");

var maxs = cpp_expression("#incl");

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var b = cpp_array(maxn);
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d%d%d", (&b[i].w), (&b[i].s), (&b[i].v));
      b[i].s += b[i].w;
      i += 1;
    }
  }
  sort(b, (b + n));
  var dp = [];
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = b[i].s;
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
