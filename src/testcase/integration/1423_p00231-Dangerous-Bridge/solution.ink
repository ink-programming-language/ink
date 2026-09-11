// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i=0;i<n;i++)");
}

var m: dynamic = cpp_array(100);

var a: dynamic = cpp_array(100);

var b: dynamic = cpp_array(100);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma(scanf("%lld", (&n)), n))
  {
    var v: dynamic = cpp_uninitialized();
    for (var i: dynamic in v)
    {
      var cnt: dynamic = 0;
      if ((cnt > 150))
      {
        puts("NG");
        cpp_goto("goto p;");
      }
    }
    puts("OK");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      scanf("%lld%lld%lld", (&m[i]), (&a[i]), (&b[i]));
      v.push_back(a[i]);
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        if (((a[j] <= i) && (i < b[j])))
        {
          cnt += m[j];
        }
      }
