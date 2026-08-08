// Translated from solution.cpp.

var N = 5050;

var a = cpp_array(N);

var b = cpp_array(N);

var pref = cpp_array(N);

var all = cpp_array(N);

func main()
{
  var t: dynamic;
  scanf("%i", (&t));
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    scanf("%i", (&n));
    {
      var i = 0;
      while ((i < n))
      {
        all[i].clear();
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        scanf("%i%i", (&a[i]), (&b[i]));
        all[a[i]].push_back(b[i]);
        i += 1;
      }
    }
    pref[0] = cpp_cast(all[0].size());
    {
      var i = 1;
      while ((i < n))
      {
        pref[i] = (pref[(i - 1)] + cpp_cast(all[i].size()));
        i += 1;
      }
    }
    var val: dynamic;
    var ans = 0;
    var cnt = 0;
    {
      var i = (n - 1);
      while ((i > 0))
      {
        for (var j in all[i])
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
