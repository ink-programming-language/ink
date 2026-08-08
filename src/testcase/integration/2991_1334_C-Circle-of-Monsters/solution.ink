// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    var v: dynamic;
    var v1: dynamic;
    var n: dynamic;
    var x: dynamic;
    var y: dynamic;
    var a: dynamic;
    var b: dynamic;
    var mn = 1000000000000;
    var mn1 = 1000000000000;
    var ans = 0;
    var ans1 = 0;
    var res = 0;
    scanf("%d", (&n));
    x = n;
    y = n;
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%lld %lld", (&a), (&b));
        v.push_back(make_pair(a, b));
        v1.push_back(make_pair(a, b));
        if ((a < mn))
        {
          mn = a;
          x = i;
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if (((v[(i + 1)].first - v[i].second) > 0))
        {
          if ((mn1 > v[i].second))
          {
            mn1 = v[i].second;
            y = (i + 1);
          }
        }
        i += 1;
      }
    }
    if (((v[0].first - v[(n - 1)].second) > 0))
    {
      if ((mn1 > v[(n - 1)].second))
      {
        mn1 = v[(n - 1)].second;
        y = 0;
      }
    }
    {
      var i = x;
      while ((i < n))
      {
        ans += v[i].first;
        if ((i == (n - 1)))
        {
          v[0].first -= v[i].second;
          if ((v[0].first < 0))
          {
            v[0].first = 0;
          }
        } else
        {
          v[(i + 1)].first -= v[i].second;
          if ((v[(i + 1)].first < 0))
          {
            v[(i + 1)].first = 0;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < x))
      {
        ans += v[i].first;
        if (((i + 1) < x))
        {
          v[(i + 1)].first -= v[i].second;
          if ((v[(i + 1)].first < 0))
          {
            v[(i + 1)].first = 0;
          }
        }
        i += 1;
      }
    }
    {
      var i = y;
      while ((i < n))
      {
        ans1 += v1[i].first;
        if ((i == (n - 1)))
        {
          v1[0].first -= v1[i].second;
          if ((v1[0].first < 0))
          {
            v1[0].first = 0;
          }
        } else
        {
          v1[(i + 1)].first -= v1[i].second;
          if ((v1[(i + 1)].first < 0))
          {
            v1[(i + 1)].first = 0;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < y))
      {
        ans1 += v1[i].first;
        if (((i + 1) < y))
        {
          v1[(i + 1)].first -= v1[i].second;
          if ((v1[(i + 1)].first < 0))
          {
            v1[(i + 1)].first = 0;
          }
        }
        i += 1;
      }
    }
    res = min(ans, ans1);
    printf("%lld\n", res);
  }
  return 0;
}
