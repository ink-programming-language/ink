// Translated from solution.cpp.

var dp = cpp_array(10005);

var a = cpp_array(105);

var b = cpp_array(105);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var cap = 0;
  var mx = 0;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      cap += a[i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&b[i]));
      mx += b[i];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= mx))
    {
      dp[i] = make_pair(-1, -1);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = mx;
        while ((j >= 0))
        {
          if ((dp[j].first == -1))
          {
            j -= 1;
            continue;
          }
          var nxt = (j + b[i]);
          if ((dp[nxt].first == -1))
          {
            dp[nxt] = make_pair((dp[j].first + 1), (dp[j].second + a[i]));
          } else if ((dp[nxt].first > (dp[j].first + 1)))
          {
            dp[nxt] = make_pair((dp[j].first + 1), (dp[j].second + a[i]));
          } else if ((dp[nxt].first == (dp[j].first + 1)))
          {
            dp[nxt].second = max(dp[nxt].second, (dp[j].second + a[i]));
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var mn = make_pair(cpp_cast(1e9), 0);
  {
    var i = cap;
    while ((i <= mx))
    {
      if ((dp[i].first == -1))
      {
        i += 1;
        continue;
      }
      if ((dp[i].first < mn.first))
      {
        mn = dp[i];
      } else if ((dp[i].first == mn.first))
      {
        mn.second = max(mn.second, dp[i].second);
      }
      i += 1;
    }
  }
  printf("%d %d\n", mn.first, (cap - mn.second));
  return 0;
}
