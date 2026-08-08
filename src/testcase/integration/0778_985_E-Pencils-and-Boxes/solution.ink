// Translated from solution.cpp.

var maxn = (5e6 + 5);

var n: dynamic;

var m: dynamic;

var d: dynamic;

var a = cpp_array(maxn);

var sum = cpp_array(maxn);

func main()
{
  scanf("%d%d%lld", (&n), (&m), (&d));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i]));
      i += 1;
    }
  }
  sort((&a[1]), (&a[(n + 1)]));
  var good = 1;
  sum[0] = 1;
  {
    var i = 1;
    var j = 1;
    while ((i <= n))
    {
      while (((a[i] - a[j]) > d))
      {
        j += 1;
      }
      good = (sum[max(-1, (i - m))] - sum[(j - 2)]);
      sum[i] = sum[(i - 1)];
      if ((good >= 1))
      {
        sum[i] += 1;
      }
      i += 1;
    }
  }
  if ((good > 0))
  {
    printf("YES");
  } else
  {
    printf("NO");
  }
}
