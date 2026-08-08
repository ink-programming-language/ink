// Translated from solution.cpp.

var mod = (1e9 + 7);

var pi = acos(-1);

var n: dynamic;

var arr = cpp_array(150005);

var mp: dynamic;

var mp2: dynamic;

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d%d", (&arr[i].first), (&arr[i].second));
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= sqrt(arr[(n - 1)].first)))
    {
      if (((arr[(n - 1)].first % i) == 0))
      {
        mp[i] = 1;
        mp[(arr[(n - 1)].first / i)] = 1;
      }
      i += 1;
    }
  }
  if ((mp.size() == 0))
  {
    mp[arr[(n - 1)].first] += 1;
  }
  var temp = mp.size();
  {
    var i = 2;
    while ((i <= sqrt(arr[(n - 1)].second)))
    {
      if (((arr[(n - 1)].second % i) == 0))
      {
        mp[i] = 1;
        mp[(arr[(n - 1)].second / i)] = 1;
      }
      i += 1;
    }
  }
  if (((mp.size() == temp) && (mp[arr[(n - 1)].second] == 0)))
  {
    mp[arr[(n - 1)].second] += 1;
  }
  for (var j in mp)
  {
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if (((arr[i].first % j.first) == 0))
        {
          j.second += 1;
        } else if (((arr[i].second % j.first) == 0))
        {
          j.second += 1;
        } else
        {
          break;
        }
        i += 1;
      }
    }
    if (((j.second >= n) && (j.first != 1)))
    {
      printf("%d", j.first);
      return 0;
    }
  }
  printf("-1");
  return 0;
}
