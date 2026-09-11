// Translated from solution.cpp.

var knapsack: dynamic = [];

var ans: dynamic = 0;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  knapsack[0][0] = 1;
  scanf("%d %d", (&n), (&a));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&t));
      {
        var k: dynamic = (i - 1);
        while ((k >= 0))
        {
          {
            var j: dynamic = 2500;
            while ((j >= t))
            {
              knapsack[(k + 1)][j] += knapsack[k][(j - t)];
              j -= 1;
            }
          }
          k -= 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans += knapsack[i][(a * i)];
      i += 1;
    }
  }
  printf("%lld", ans);
}
