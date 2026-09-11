// Translated from solution.cpp.

var ans: dynamic = cpp_uninitialized();

var M: dynamic = 1e5;

var vis: dynamic = cpp_array((M + 5));

var isPrime: dynamic = cpp_array((M + 5));

var factor: dynamic = cpp_array((M + 5));

func getPrime() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= M))
    {
      isPrime[i] = 1;
      i += 1;
    }
  }
  isPrime[0] = cpp_assign(isPrime[1], "=", 0);
  {
    var i: dynamic = 2;
    while (((i * i) <= M))
    {
      if (isPrime[i])
      {
        {
          var j: dynamic = (i * i);
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
    var i: dynamic = 2;
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

func main() -> dynamic
{
  getPrime();
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 3;
    while ((i <= n))
    {
      if ((!isPrime[i]))
      {
        i += 1;
        continue;
      }
      var co: dynamic = cpp_uninitialized();
      {
        var j: dynamic = i;
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
          var j: dynamic = 1;
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
        var tmp: dynamic = co;
        co.clear();
        var f: dynamic = 0;
        {
          var j: dynamic = 0;
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
          var j: dynamic = 1;
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
  var co: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 2;
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
    var i: dynamic = 1;
    while ((i < co.size()))
    {
      ans.push_back(pair(co[(i - 1)], co[i]));
      i += 2;
    }
  }
  printf("%d\n", ans.size());
  {
    var i: dynamic = 0;
    while ((i < ans.size()))
    {
      printf("%d %d\n", ans[i].first, ans[i].second);
      i += 1;
    }
  }
}
