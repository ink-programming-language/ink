// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a: dynamic;
  var m: dynamic;
  scanf("%lld", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%lld", (&a));
      m[a] += 1;
      i += 1;
    }
  }
  var v: dynamic;
  for (var e in m)
  {
    v.push_back(e.second);
  }
  sort(v.begin(), v.end());
  reverse(v.begin(), v.end());
  var ans = v[0];
  var tmp: dynamic;
  var now = v[0];
  var kal = 2;
  {
    var i = 1;
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
