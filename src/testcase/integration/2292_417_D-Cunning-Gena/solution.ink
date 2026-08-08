// Translated from solution.cpp.

var PINF = numeric_limits.max();

var M = (1E9 + 7);

var EPS = 1E-9;

class frnd
{
  var solved: dynamic;
  var cost: dynamic;
  var monitors: dynamic;
}

func cmp(a: dynamic, b: dynamic)
{
  return (a.monitors < b.monitors);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  var b: dynamic;
  read(n, m, b);
  var friends = cpp_construct((n + 1));
  {
    var i = 0;
    while ((i < n))
    {
      var cnt: dynamic;
      read(friends[i].cost, friends[i].monitors, cnt);
      {
        var j = 0;
        while ((j < cnt))
        {
          var problem: dynamic;
          read(problem);
          friends[i].solved |= ((1 << ((problem - 1))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(friends.begin(), friends.end(), cmp);
  var dp = cpp_construct((1 << m), PINF);
  var full_mask = (((1 << m)) - 1);
  var ans = PINF;
  dp[0] = 0;
  {
    var f = 1;
    while ((f <= n))
    {
      {
        var mask = 0;
        while ((mask < ((1 << m))))
        {
          if ((dp[mask] < PINF))
          {
            var new_mask = (mask | friends[f].solved);
            dp[new_mask] = min(dp[new_mask], (dp[mask] + friends[f].cost));
          }
          mask += 1;
        }
      }
      if ((dp[full_mask] < PINF))
      {
        ans = min(ans, (dp[full_mask] + (b * friends[f].monitors)));
      }
      f += 1;
    }
  }
  if ((ans == PINF))
  {
    write(-1, cpp_char("\n"));
  } else
  {
    write(ans, "\n");
  }
  return 0;
}
