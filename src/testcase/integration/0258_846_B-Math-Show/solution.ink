// Translated from solution.cpp.

var INF: dynamic = numeric_limits.max();

var LLINF: dynamic = numeric_limits.max();

var ULLINF: dynamic = numeric_limits.max();

var PI: dynamic = acos(-1.0);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_array(50);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  read(n, k, M);
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      read(t[i]);
      sum += t[i];
      i += 1;
    }
  }
  sort(t, (t + k));
  var ans: dynamic = 0;
  {
    var s: dynamic = 0;
    while ((s <= n))
    {
      if (((sum * s) > M))
      {
        break;
      }
      var p: dynamic = (n - s);
      var T: dynamic = (M - (sum * s));
      var cur: dynamic = (((k + 1)) * s);
      {
        var i: dynamic = 0;
        while ((i < k))
        {
          var take: dynamic = min((T / t[i]), p);
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
