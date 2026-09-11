// Translated from solution.cpp.

var PINF: dynamic = numeric_limits.max();

var M: dynamic = (1E9 + 7);

var EPS: dynamic = 1E-9;

class frnd
{
  var solved: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  var monitors: dynamic = cpp_uninitialized();
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.monitors < b.monitors);
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(n, m, b);
  var friends: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var cnt: dynamic = cpp_uninitialized();
      read(friends[i].cost, friends[i].monitors, cnt);
      {
        var j: dynamic = 0;
        while ((j < cnt))
        {
          var problem: dynamic = cpp_uninitialized();
          read(problem);
          friends[i].solved |= ((1 << ((problem - 1))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(friends.begin(), friends.end(), cmp);
  var dp: dynamic = cpp_construct((1 << m), PINF);
  var full_mask: dynamic = (((1 << m)) - 1);
  var ans: dynamic = PINF;
  dp[0] = 0;
  {
    var f: dynamic = 1;
    while ((f <= n))
    {
      {
        var mask: dynamic = 0;
        while ((mask < ((1 << m))))
        {
          if ((dp[mask] < PINF))
          {
            var new_mask: dynamic = (mask | friends[f].solved);
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
