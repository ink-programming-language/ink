// Translated from solution.cpp.

var ep: dynamic = cpp_array(200005);

var dp: dynamic = cpp_array(200005);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&k), (&m));
  while (cpp_update(n, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    scanf("%d%d", (&a), (&b));
    ep[a] += 1;
    ep[(b + 1)] -= 1;
  }
  {
    var i: dynamic = 1;
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
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    scanf("%d%d", (&a), (&b));
    printf("%d\n", (dp[b] - dp[(a - 1)]));
  }
}
