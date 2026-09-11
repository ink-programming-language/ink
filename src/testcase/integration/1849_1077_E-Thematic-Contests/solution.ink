// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%lld", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lld", (&a));
      m[a] += 1;
      i += 1;
    }
  }
  var v: dynamic = cpp_uninitialized();
  for (var e: dynamic in m)
  {
    v.push_back(e.second);
  }
  sort(v.begin(), v.end());
  reverse(v.begin(), v.end());
  var ans: dynamic = v[0];
  var tmp: dynamic = cpp_uninitialized();
  var now: dynamic = v[0];
  var kal: dynamic = 2;
  {
    var i: dynamic = 1;
    while ((i < v.size()))
    {
      now >>= 1;
      now = min(now, v[i]);
      if ((!now))
      {
        break;
      }
      kal <<= 1;
      tmp = (now * ((kal - 1)));
      ans = max(ans, tmp);
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
