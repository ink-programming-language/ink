// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i=0;i<n;i++)");
}

var a: dynamic = cpp_array(200000);

func main() -> dynamic
{
  var d: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
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
    var cnt: dynamic = 0;
    printf("%lld\n", cnt);
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var k: dynamic = cpp_uninitialized();
      scanf("%lld", (&k));
      var p: dynamic = lower_bound(a, ((a + n) + 2), k);
      cnt += min(((*p) - k), (k - (*((p - 1)))));
    }
