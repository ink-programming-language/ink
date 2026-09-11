// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; (i) < (n); ++(i))");
}

func REP3(i: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (m); (i) < (n); ++(i))");
}

func REP_R(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (int)(n) - 1; (i) >= 0; --(i))");
}

func ALL(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/s");
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  a = min(a, b);
}

func make_table(s: dynamic) -> dynamic
{
  var n: dynamic = s.length();
  var next: dynamic = cpp_construct((n + 2));
  next[n][0] = n;
  next[n][1] = n;
  next[(n + 1)][0] = n;
  next[(n + 1)][1] = n;
  return next;
}

func solve(p: dynamic, q: dynamic) -> dynamic
{
  var next_p: dynamic = make_table(p);
  var next_q: dynamic = make_table(q);
  var dp: dynamic = cpp_construct((p.size() + 2), vector((q.size() + 2), INT_MAX));
  dp[0][0] = 0;
  REP(i, (p.size() + 2));
  {
    REP(j, (q.size() + 2));
    if ((dp[i][j] != INT_MAX))
    {
      cpp_statement("REP (c, 2)");
      {
        chmin(dp[(next_p[i][c] + 1)][(next_q[j][c] + 1)], (dp[i][j] + 1));
      }
    }
  }
  var pred: dynamic = cpp_construct((p.size() + 2), vector((q.size() + 2), false));
  pred[(p.size() + 1)][(q.size() + 1)] = true;
  REP_R(i, (p.size() + 2));
  {
    REP_R(j, (q.size() + 2));
    if ((dp[i][j] != INT_MAX))
    {
      cpp_statement("REP (c, 2)");
      {
        var ni: dynamic = (next_p[i][c] + 1);
        var nj: dynamic = (next_q[j][c] + 1);
        if (cpp_binary(pred[ni][nj], "and", (dp[ni][nj] == (dp[i][j] + 1))))
        {
          pred[i][j] = true;
        }
      }
    }
  }
  assert(pred[0][0]);
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    var j: dynamic = 0;
    while (cpp_binary((i <= p.size()), "or", (j <= q.size())))
    {
      var c: dynamic = 0;
      while (cpp_binary((c < 2), "and", cpp_unary("not", (cpp_binary(pred[(next_p[i][c] + 1)][(next_q[j][c] + 1)], "and", (dp[(next_p[i][c] + 1)][(next_q[j][c] + 1)] == (dp[i][j] + 1)))))))
      {
        c += 1;
      }
      assert((c != 2));
      s += (c + cpp_char("0"));
      i = (next_p[i][c] + 1);
      j = (next_q[j][c] + 1);
    }
  }
  return s;
}

func main() -> dynamic
{
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(p, q);
  write(solve(p, q), "\n");
  return 0;
}

func REP_R(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    next[i] = next[(i + 1)];
    next[i][(s[i] - cpp_char("0"))] = i;
  }
