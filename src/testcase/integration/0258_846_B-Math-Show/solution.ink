// Translated from solution.cpp.

var INF = numeric_limits.max();

var LLINF = numeric_limits.max();

var ULLINF = numeric_limits.max();

var PI = acos(-1.0);

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t = cpp_array(50);
  var n: dynamic;
  var k: dynamic;
  var M: dynamic;
  read(n, k, M);
  var sum = 0;
  {
    var i = 0;
    while ((i < k))
    {
      read(t[i]);
      sum += t[i];
      i += 1;
    }
  }
  sort(t, (t + k));
  var ans = 0;
  {
    var s = 0;
    while ((s <= n))
    {
      if (((sum * s) > M))
      {
        break;
      }
      var p = (n - s);
      var T = (M - (sum * s));
      var cur = (((k + 1)) * s);
      {
        var i = 0;
        while ((i < k))
        {
          var take = min((T / t[i]), p);
          cur += take;
          T -= (take * t[i]);
          i += 1;
        }
      }
      ans = max(ans, cur);
      s += 1;
    }
  }
  write(ans);
  return 0;
}
