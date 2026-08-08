// Translated from solution.cpp.

var ans: dynamic;

var M = 1e5;

var vis = cpp_array((M + 5));

var isPrime = cpp_array((M + 5));

var factor = cpp_array((M + 5));

func getPrime()
{
  {
    var i = 0;
    while ((i <= M))
    {
      isPrime[i] = 1;
      i += 1;
    }
  }
  isPrime[0] = cpp_assign(isPrime[1], "=", 0);
  {
    var i = 2;
    while (((i * i) <= M))
    {
      if (isPrime[i])
      {
        {
          var j = (i * i);
          while ((j <= M))
          {
            isPrime[j] = 0;
            if ((factor[j] == 0))
            {
              factor[j] = i;
            }
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= M))
    {
      if ((factor[i] == 0))
      {
        factor[i] = i;
      }
      i += 1;
    }
  }
}

func main()
{
  getPrime();
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 3;
    while ((i <= n))
    {
      if ((!isPrime[i]))
      {
        i += 1;
        continue;
      }
      var co: dynamic;
      {
        var j = i;
        while ((j <= n))
        {
          if (vis[j])
          {
            j += i;
            continue;
          }
          co.push_back(j);
          j += i;
        }
      }
      if (((co.size() % 2) == 0))
      {
        {
          var j = 1;
          while ((j < co.size()))
          {
            ans.push_back(pair(co[(j - 1)], co[j]));
            vis[co[(j - 1)]] = cpp_assign(vis[co[j]], "=", 1);
            j += 2;
          }
        }
      } else
      {
        if ((co.size() == 1))
        {
          i += 1;
          continue;
        }
        var tmp = co;
        co.clear();
        var f = 0;
        {
          var j = 0;
          while ((j < tmp.size()))
          {
            if ((((tmp[j] % 2) == 0) && (!f)))
            {
              f = 1;
              j += 1;
              continue;
            } else
            {
              co.push_back(tmp[j]);
            }
            j += 1;
          }
        }
        {
          var j = 1;
          while ((j < co.size()))
          {
            ans.push_back(pair(co[(j - 1)], co[j]));
            vis[co[(j - 1)]] = cpp_assign(vis[co[j]], "=", 1);
            j += 2;
          }
        }
      }
      i += 1;
    }
  }
  var co: dynamic;
  {
    var i = 2;
    while ((i <= n))
    {
      if (vis[i])
      {
        i += 2;
        continue;
      }
      co.push_back(i);
      i += 2;
    }
  }
  {
    var i = 1;
    while ((i < co.size()))
    {
      ans.push_back(pair(co[(i - 1)], co[i]));
      i += 2;
    }
  }
  printf("%d\n", ans.size());
  {
    var i = 0;
    while ((i < ans.size()))
    {
      printf("%d %d\n", ans[i].first, ans[i].second);
      i += 1;
    }
  }
}
