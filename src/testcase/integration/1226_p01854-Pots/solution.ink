// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func each(itr: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof(c.begin()) itr=c.begin(); itr!=c.end(); ++itr)");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

var pb: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_array(200);

var s: dynamic = cpp_array(20, 200);

var h: dynamic = cpp_array(20, 200);

var dp: dynamic = cpp_array(201, 201);

func dfs(now: dynamic, r: dynamic) -> dynamic
{
  if ((now == n))
  {
    return 0;
  }
  if ((dp[now][r] >= 0))
  {
    return dp[now][r];
  }
  var ret: dynamic = 0;
  rep(i, (r + 1));
  {
    var add: dynamic = 0;
    var tr: dynamic = i;
    ret = max(ret, (dfs((now + 1), (r - i)) + add));
  }
  return cpp_assign(dp[now][r], "=", ret);
}

func main() -> dynamic
{
  scanf(" %d %d", (&n), (&m));
  fill(dp[0], dp[201], -1.0);
  printf("%.10f\n", dfs(0, m));
  return 0;
}

func rep(argument_0: dynamic, now: dynamic) -> dynamic
{
      var d: dynamic = (s[now][j] * h[now][j]);
      if (((tr - d) < 0))
      {
        add += (cpp_cast(tr) / s[now][j]);
        break;
      }
      tr -= d;
      add += h[now][j];
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    scanf(" %d", (&k[i]));
    rep(j, k[i]);
    scanf(" %d %d", (&s[i][j]), (&h[i][j]));
  }
