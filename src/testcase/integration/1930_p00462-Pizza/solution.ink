// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(ll i=0;i<n;i++)");
}

var a = cpp_array(200000);

func main()
{
  var d: dynamic;
  var n: dynamic;
  var m: dynamic;
  while (cpp_comma(scanf("%lld", (&d)), d))
  {
    scanf("%lld%lld", (&n), (&m));
    rep(i, (n - 1));
    scanf("%lld", (&a[i]));
    a[(n - 1)] = 0;
    a[n] = d;
    sort(a, ((a + n) + 1));
    a[(n + 1)] = (a[(n - 1)] - d);
    sort(a, ((a + n) + 2));
    var cnt = 0;
    printf("%lld\n", cnt);
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var k: dynamic;
      scanf("%lld", (&k));
      var p = lower_bound(a, ((a + n) + 2), k);
      cnt += min(((*p) - k), (k - (*((p - 1)))));
    }
