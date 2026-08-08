// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(k); i<(int)n; ++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func FORIT(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var h: dynamic;
  var k: dynamic;
  while (cpp_comma(((((cin >> n) >> m) >> h) >> k), n))
  {
    REP(i, n);
    read(s[i]);
    sort(stc.begin(), stc.end());
    var memo = cpp_array(2, 100000);
    var now = cpp_array(100000);
    var point = cpp_array(1000);
    REP(i, n)[i] = i;
    var fstans = 0;
    var subans = 0;
    REP(i, n)[now[i]] = s[i];
    REP(i, k) += point[i];
    write((fstans + subans), "\n");
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      stc[i] = P(b, (a - 1));
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      memo[i][0] = now[stc[i].second];
      memo[i][1] = now[(stc[i].second + 1)];
      swap(now[stc[i].second], now[(stc[i].second + 1)]);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
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
