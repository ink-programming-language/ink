// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(k); i<(int)n; ++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func FORIT(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  while (cpp_comma(((((cin >> n) >> m) >> h) >> k), n))
  {
    REP(i, n);
    read(s[i]);
    sort(stc.begin(), stc.end());
    var memo: dynamic = cpp_array(2, 100000);
    var now: dynamic = cpp_array(100000);
    var point: dynamic = cpp_array(1000);
    REP(i, n)[i] = i;
    var fstans: dynamic = 0;
    var subans: dynamic = 0;
    REP(i, n)[now[i]] = s[i];
    REP(i, k) += point[i];
    write((fstans + subans), "\n");
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      stc[i] = P(b, (a - 1));
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      memo[i][0] = now[stc[i].second];
      memo[i][1] = now[(stc[i].second + 1)];
      swap(now[stc[i].second], now[(stc[i].second + 1)]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if (((memo[i][0] < k) && (memo[i][1] >= k)))
      {
        subans = min(subans, ((-point[memo[i][0]]) + point[memo[i][1]]));
      }
      if (((memo[i][1] < k) && (memo[i][0] >= k)))
      {
        subans = min(subans, ((-point[memo[i][1]]) + point[memo[i][0]]));
      }
    }
