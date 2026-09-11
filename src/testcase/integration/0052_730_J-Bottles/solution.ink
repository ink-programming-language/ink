// Translated from solution.cpp.

var dp: dynamic = cpp_array(10005);

var a: dynamic = cpp_array(105);

var b: dynamic = cpp_array(105);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var cap: dynamic = 0;
  var mx: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      cap += a[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&b[i]));
      mx += b[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= mx))
    {
      dp[i] = make_pair(-1, -1);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = mx;
        while ((j >= 0))
        {
          if ((dp[j].first == -1))
          {
            j -= 1;
            continue;
          }
          var nxt: dynamic = (j + b[i]);
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
  var mn: dynamic = make_pair(cpp_cast(1e9), 0);
  {
    var i: dynamic = cap;
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
