// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0; i<n; i++)");
}

var dp = cpp_array(110, 110);

var dp2 = cpp_array(110, 110);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var L: dynamic;
  var K: dynamic;
  var A: dynamic;
  var H: dynamic;
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
    var a: dynamic;
    var h: dynamic;
    if ((dp2[h][a] > 1e8))
    {
      write("Help!", "\n");
    } else
    {
      var t = dp2[h][a];
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      read(a, b, c);
      dp[a][b] = cpp_assign(dp[b][a], "=", c);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        dp2[i][j] = (if ((dp[ll[i]][ll[j]] > m)) 1e9 else dp[ll[i]][ll[j]]);
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    }

func rep(argument_0: dynamic, argument_1: dynamic)
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
