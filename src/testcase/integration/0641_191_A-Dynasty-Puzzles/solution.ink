// Translated from solution.cpp.

var dp: dynamic = cpp_array(26, 26);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var l: dynamic = 0;
  var p: dynamic = 0;
  var i: dynamic = cpp_uninitialized();
  var flag: dynamic = 0;
  var k: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var d: dynamic = 0;
  var q: dynamic = 0;
  var r: dynamic = 0;
  read(n);
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s);
      var start: dynamic = cpp_uninitialized();
      var last: dynamic = cpp_uninitialized();
      k = cpp_cast(s.size());
      start = (s[0] - cpp_char("a"));
      last = (s[(k - 1)] - cpp_char("a"));
      {
        var j: dynamic = 0;
        while ((j < 26))
        {
          if ((dp[j][start] == 0))
          {
            j += 1;
            continue;
          }
          dp[j][last] = max(dp[j][last], (dp[j][start] + k));
          j += 1;
        }
      }
      dp[start][last] = max(dp[start][last], k);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      p = max(p, dp[i][i]);
      i += 1;
    }
  }
  write(p);
  return 0;
}
