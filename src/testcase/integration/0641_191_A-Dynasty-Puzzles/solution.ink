// Translated from solution.cpp.

var dp = cpp_array(26, 26);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  var l = 0;
  var p = 0;
  var i: dynamic;
  var flag = 0;
  var k: dynamic;
  var t: dynamic;
  var d = 0;
  var q = 0;
  var r = 0;
  read(n);
  var s: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(s);
      var start: dynamic;
      var last: dynamic;
      k = cpp_cast(s.size());
      start = (s[0] - cpp_char("a"));
      last = (s[(k - 1)] - cpp_char("a"));
      {
        var j = 0;
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
    var i = 0;
    while ((i < 26))
    {
      p = max(p, dp[i][i]);
      i += 1;
    }
  }
  write(p);
  return 0;
}
