// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0; i<n; i++)");
}

var dp: dynamic = cpp_array(110, 110);

var dp2: dynamic = cpp_array(110, 110);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  var H: dynamic = cpp_uninitialized();
  while (cpp_comma(((((((cin >> n) >> m) >> L) >> K) >> A) >> H), n))
  {
    cpp_statement("rep(i,n) rep(j,n) dp[i][j] = dp2[i][j] = 1e9; vector<int> ll(L); rep(i,L)");
    read(ll[i]);
    ll.push_back(A);
    ll.push_back(H);
    L += 2;
    sort(ll.begin(), ll.end());
    rep(i, n)[i][i] = 0;
    rep(k, n);
    rep(i, n);
    rep(j, n)[i][j] = min(dp[i][j], (dp[i][k] + dp[k][j]));
    rep(i, L)[i][i] = 0;
    rep(k, L);
    rep(i, L);
    rep(j, L)[i][j] = min(dp2[i][j], (dp2[i][k] + dp2[k][j]));
    var a: dynamic = cpp_uninitialized();
    var h: dynamic = cpp_uninitialized();
    if ((dp2[h][a] > 1e8))
    {
      write("Help!", "\n");
    } else
    {
      var t: dynamic = dp2[h][a];
      if ((t < m))
      {
        write(t, "\n");
      } else
      {
        write((t + ((t - m))), "\n");
      }
    }
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      read(a, b, c);
      dp[a][b] = cpp_assign(dp[b][a], "=", c);
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        dp2[i][j] = ( ((dp[ll[i]][ll[j]] > m)) ? 1e9 : dp[ll[i]][ll[j]]);
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((ll[i] == A))
      {
        a = i;
      }
      if ((ll[i] == H))
      {
        h = i;
      }
    }
