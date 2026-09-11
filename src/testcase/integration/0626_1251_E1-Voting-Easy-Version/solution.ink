// Translated from solution.cpp.

var N: dynamic = 5050;

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var pref: dynamic = cpp_array(N);

var all: dynamic = cpp_array(N);

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%i", (&t));
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    scanf("%i", (&n));
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        all[i].clear();
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        scanf("%i%i", (&a[i]), (&b[i]));
        all[a[i]].push_back(b[i]);
        i += 1;
      }
    }
    pref[0] = cpp_cast(all[0].size());
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        pref[i] = (pref[(i - 1)] + cpp_cast(all[i].size()));
        i += 1;
      }
    }
    var val: dynamic = cpp_uninitialized();
    var ans: dynamic = 0;
    var cnt: dynamic = 0;
    {
      var i: dynamic = (n - 1);
      while ((i > 0))
      {
        for (var j: dynamic in all[i])
        {
          val.insert(j);
        }
        while (((pref[(i - 1)] + cnt) < i))
        {
          cnt += 1;
          ans += (*val.begin());
          val.erase(val.begin());
        }
        i -= 1;
      }
    }
    printf("%lld\n", ans);
  }
  return 0;
}
