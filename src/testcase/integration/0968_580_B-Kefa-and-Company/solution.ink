// Translated from solution.cpp.

var INF: dynamic = 1e18;

var MAXN: dynamic = (100000 + 1000);

class node
{
  var m: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  func operator_less(b: dynamic) -> dynamic
  {
      return (m < b.m);
    }
}

var num: dynamic = cpp_array(MAXN);

var sum: dynamic = cpp_array(MAXN);

var M: dynamic = cpp_array(MAXN);

var n: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while ((scanf("%d%d", (&n), (&d)) != EOF))
  {
    {
      var i: dynamic = 1;
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
      var i: dynamic = 1;
      while ((i <= n))
      {
        M[i] = num[i].m;
        i += 1;
      }
    }
    M[(n + 1)] = INF;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        sum[i] = (sum[(i - 1)] + num[i].s);
        i += 1;
      }
    }
    sum[(n + 1)] = sum[n];
    var ans: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= (n + 1)))
      {
        var s: dynamic = (i - 1);
        var e: dynamic = (((cpp_cast(upper_bound((M + 1), ((M + n) + 2), ((cpp_cast(M[i]) + cpp_cast(d)) - 1))) - cpp_cast(((M + 1))))) / cpp_sizeof(dynamic));
        var temp: dynamic = (sum[e] - sum[s]);
        ans = max(ans, temp);
        i += 1;
      }
    }
    printf("%I64d\n", ans);
  }
}
