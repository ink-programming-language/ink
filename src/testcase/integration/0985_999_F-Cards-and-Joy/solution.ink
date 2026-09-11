// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var joy: dynamic = cpp_array(15);

var dp: dynamic = cpp_array(5005, 505);

func solve(play: dynamic, cards: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= play))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= play))
    {
      {
        var j: dynamic = 1;
        while ((j <= cards))
        {
          {
            var l: dynamic = 1;
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

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  read(n, k);
  var c: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (n * k)))
    {
      read(x);
      c[x] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      f[x] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      read(joy[i]);
      i += 1;
    }
  }
  joy[0] = 0;
  var ans: dynamic = 0;
  {
    var it: dynamic = f.begin();
    while ((it != f.end()))
    {
      ans += solve(it->second, min((k * (it->second)), c[it->first]));
      it += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
