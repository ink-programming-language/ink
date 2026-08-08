// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var joy = cpp_array(15);

var dp = cpp_array(5005, 505);

func solve(play: dynamic, cards: dynamic)
{
  {
    var i = 0;
    while ((i <= play))
    {
      {
        var j = 0;
        while ((j <= cards))
        {
          dp[i][j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= play))
    {
      {
        var j = 1;
        while ((j <= cards))
        {
          {
            var l = 1;
            while ((l <= min(k, j)))
            {
              dp[i][j] = max(dp[i][j], (dp[(i - 1)][(j - l)] + joy[l]));
              l += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return dp[play][cards];
}

func main()
{
  var x: dynamic;
  read(n, k);
  var c: dynamic;
  var f: dynamic;
  {
    var i = 0;
    while ((i < (n * k)))
    {
      read(x);
      c[x] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(x);
      f[x] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      read(joy[i]);
      i += 1;
    }
  }
  joy[0] = 0;
  var ans = 0;
  {
    var it = f.begin();
    while ((it != f.end()))
    {
      ans += solve(it->second, min((k * (it->second)), c[it->first]));
      it += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
