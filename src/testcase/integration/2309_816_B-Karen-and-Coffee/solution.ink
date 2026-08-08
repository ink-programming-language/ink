// Translated from solution.cpp.

var ep = cpp_array(200005);

var dp = cpp_array(200005);

func main()
{
  var n: dynamic;
  var k: dynamic;
  var m: dynamic;
  scanf("%d%d%d", (&n), (&k), (&m));
  while (cpp_update(n, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    scanf("%d%d", (&a), (&b));
    ep[a] += 1;
    ep[(b + 1)] -= 1;
  }
  {
    var i = 1;
    while ((i <= 200002))
    {
      ep[i] += ep[(i - 1)];
      if ((ep[i] >= k))
      {
        dp[i] += 1;
      }
      dp[i] += dp[(i - 1)];
      i += 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    scanf("%d%d", (&a), (&b));
    printf("%d\n", (dp[b] - dp[(a - 1)]));
  }
}
