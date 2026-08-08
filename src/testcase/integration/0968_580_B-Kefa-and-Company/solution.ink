// Translated from solution.cpp.

var INF = 1e18;

var MAXN = (100000 + 1000);

class node
{
  var m: dynamic;
  var s: dynamic;
  func operator_less(b: dynamic)
  {
      return (m < b.m);
    }
}

var num = cpp_array(MAXN);

var sum = cpp_array(MAXN);

var M = cpp_array(MAXN);

var n: dynamic;

var d: dynamic;

func main()
{
  while ((scanf("%d%d", (&n), (&d)) != EOF))
  {
    {
      var i = 1;
      while ((i <= n))
      {
        scanf("%I64d%I64d", (&num[i].m), (&num[i].s));
        i += 1;
      }
    }
    sort((num + 1), ((num + n) + 1));
    sum[0] = 0;
    M[0] = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        M[i] = num[i].m;
        i += 1;
      }
    }
    M[(n + 1)] = INF;
    {
      var i = 1;
      while ((i <= n))
      {
        sum[i] = (sum[(i - 1)] + num[i].s);
        i += 1;
      }
    }
    sum[(n + 1)] = sum[n];
    var ans = 0;
    {
      var i = 1;
      while ((i <= (n + 1)))
      {
        var s = (i - 1);
        var e = (((cpp_cast(upper_bound((M + 1), ((M + n) + 2), ((cpp_cast(M[i]) + cpp_cast(d)) - 1))) - cpp_cast(((M + 1))))) / cpp_sizeof(dynamic));
        var temp = (sum[e] - sum[s]);
        ans = max(ans, temp);
        i += 1;
      }
    }
    printf("%I64d\n", ans);
  }
}
