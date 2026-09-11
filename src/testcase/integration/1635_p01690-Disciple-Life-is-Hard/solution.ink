// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

func FOR(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func ALL(c: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #i");
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <iostr");
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <iostr");
}

func valid(y: dynamic, x: dynamic, h: dynamic, w: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #i");
}

var INF: dynamic = (1 << 29);

var EPS: dynamic = 1e-8;

var PI: dynamic = acos(-1);

var e: dynamic = cpp_array(100);

var c: dynamic = cpp_array(100);

var h: dynamic = cpp_array(100);

var a: dynamic = cpp_array(100);

var dp: dynamic = cpp_array(101, 101);

var dp_tr: dynamic = cpp_array(101, 101);

var dp_d: dynamic = cpp_array(10000);

func main() -> dynamic
{
  var S: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  var U: dynamic = cpp_uninitialized();
  var N: dynamic = cpp_uninitialized();
  var O: dynamic = cpp_uninitialized();
  var D: dynamic = cpp_uninitialized();
  while (((((((cin >> S) >> T) >> U) >> N) >> O) >> D))
  {
    memset(dp_tr, -1, cpp_sizeof((dp_tr)));
    dp_tr[0][0] = 0;
    memset(dp_d, 0, cpp_sizeof((dp_d)));
    memset(dp, -1, cpp_sizeof((dp)));
    dp[0][S] = 0;
    write((*max_element(dp[D], ((dp[D] + S) + 1))), "\n");
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(e[i], c[i]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(h[i], a[i]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      {
        var j: dynamic = (T - 1);
        while ((j >= 0))
        {
          cpp_statement("REP(k,101)");
          {
            if ((dp_tr[j][k] == -1))
            {
              j -= 1;
              continue;
            }
            if (((k + e[i]) <= 100))
            {
              chmax(dp_tr[(j + 1)][(k + e[i])], (dp_tr[j][k] + c[i]));
            }
          }
          j -= 1;
        }
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      REP(k, 10001);
      {
        if (((k + a[i]) <= 10000))
        {
          chmax(dp_d[(k + a[i])], (dp_d[k] + h[i]));
        }
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      cpp_statement("REP(j,S+1)");
      {
        if ((dp[i][j] == -1))
        {
          continue;
        }
        REP(k, (j + 1));
        {
          if ((dp_tr[U][k] == -1))
          {
            continue;
          }
          chmax(dp[(i + 1)][min(S, ((j - k) + O))], (dp[i][j] + dp_d[dp_tr[U][k]]));
        }
      }
    }
