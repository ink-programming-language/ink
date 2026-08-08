// Translated from solution.cpp.

var knapsack = [];

var ans = 0;

func main()
{
  var n: dynamic;
  var a: dynamic;
  var t: dynamic;
  knapsack[0][0] = 1;
  scanf("%d %d", (&n), (&a));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&t));
      {
        var k = (i - 1);
        while ((k >= 0))
        {
          {
            var j = 2500;
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
    var i = 1;
    while ((i <= n))
    {
      ans += knapsack[i][(a * i)];
      i += 1;
    }
  }
  printf("%lld", ans);
}
