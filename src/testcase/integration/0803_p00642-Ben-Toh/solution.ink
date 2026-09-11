// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func rp(i: dynamic, c: dynamic) -> dynamic
{
  return cpp_expression("#include <iostrea");
}

func fr(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin()) i=(c).begin();i!=(c).end();i++)");
}

var mp: dynamic = cpp_expression("#include");

var pb: dynamic = cpp_expression("#include");

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #");
}

func dbg(x: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #inclu");
}

var inf: dynamic = (1 << 28);

var INF: dynamic = 1e10;

var EPS: dynamic = 1e-9;

var n: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(30, 100001);

var ans: dynamic = cpp_array(100001);

func main() -> dynamic
{
  var pw: dynamic = cpp_array(30);
  pw[0] = 1;
  rep(i, 29)[(i + 1)] = (pw[i] * 0.5);
  fill_n(ans, 100001, 0);
  rep(i, 100001);
  rep(j, 30)[i][j] = 0;
  dp[0][0] = 1;
  {
    var i: dynamic = 1;
    while ((i < 100001))
    {
      cpp_statement("rep(j,30) { dp[i][0]+=dp[i-1][j]*(1-pw[j]); if(j<29)dp[i][j+1]+=dp[i-1][j]*pw[j]; } ans[i]=ans[i-1]; rep(j,29)");
      ans[i] += dp[i][(j + 1)];
      i += 1;
    }
  }
  while (cpp_comma((cin >> n), n))
  {
    printf("%.3f\n", ans[n]);
  }
  return 0;
}
