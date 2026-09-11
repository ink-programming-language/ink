// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll (i) = (0);(i) < (n);++i)");
}

func REV(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll (i) = (n) - 1;(i) >= 0;--i)");
}

var PB: dynamic = cpp_expression("#include");

var EB: dynamic = cpp_expression("#include <bi");

var MP: dynamic = cpp_expression("#include");

var FI: dynamic = cpp_expression("#incl");

var SE: dynamic = cpp_expression("#inclu");

func SHOW1d(v: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("{REP(WW,n)cerr << v[WW] << ' ';cerr << endl << endl;}");
}

func SHOW2d(v: dynamic, WW: dynamic, HH: dynamic) -> dynamic
{
  cpp_macro("{REP(W_,WW){REP(H_,HH)cerr << v[W_][H_] << ' ';cerr << endl;}cerr << endl;}");
}

func ALL(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

var Decimal: dynamic = cpp_expression("#include <bits/stdc++.h");

var INF: dynamic = cpp_expression("#include <");

var LLINF: dynamic = cpp_expression("#include <bits/stdc++");

var MOD: dynamic = cpp_expression("#include");

var ans: dynamic = LLINF;

var dp: dynamic = cpp_array(11111, 2);

func init() -> dynamic
{
  cpp_statement("REP(i, 2)REP(j, 11111)dp[i][j] = -LLINF; REP(i, 2)");
  dp[i][0] = LLINF;
}

func check(c_max: dynamic, a: dynamic, b: dynamic, w: dynamic) -> dynamic
{
  var deq: dynamic = cpp_uninitialized();
  var r: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= w))
    {
      while ((r < i))
      {
        r += 1;
      }
      while (((r <= w) && (((c_max - dp[b][r]) > (r - i)))))
      {
        while (((!deq.empty()) && (deq.back().FI < dp[b][r])))
        {
          deq.pop_back();
        }
        if ((dp[b][r] != (-LLINF)))
        {
          deq.push_back(MP(dp[b][r], r));
        }
        r += 1;
      }
      while (((!deq.empty()) && (deq.front().SE < i)))
      {
        deq.pop_front();
      }
      if ((dp[a][i] == (-LLINF)))
      {
        i += 1;
        continue;
      }
      if ((!deq.empty()))
      {
        ans = min(ans, (c_max - min(dp[a][i], deq.front().FI)));
      }
      if (((r <= w) && (dp[b][r] != (-INF))))
      {
        ans = min(ans, max((r - i), (c_max - dp[a][i])));
      }
      i += 1;
    }
  }
}

func query(cat: dynamic, type_cpp: dynamic, w: dynamic) -> dynamic
{
  REV(i, 11111);
  {
    if (((i + cat.SE) >= 11111))
    {
      continue;
    }
    if ((dp[type_cpp][i] == (-LLINF)))
    {
      continue;
    }
    dp[type_cpp][(i + cat.SE)] = max(dp[type_cpp][(i + cat.SE)], min(dp[type_cpp][i], cat.FI));
  }
  check(cat.FI, 0, 1, w);
  check(cat.FI, 1, 0, w);
}

func main() -> dynamic
{
  cin.tie(0);
  cout.tie(0);
  ios.sync_with_stdio(false);
  init();
  var na: dynamic = cpp_uninitialized();
  var nb: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  read(na, nb, w);
  var v: dynamic = cpp_uninitialized();
  sort(ALL(v));
  REP(i, v.size());
  {
    query(v[i].FI, v[i].SE, w);
  }
  write(ans, "\n");
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var m: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(m, c);
    v.EB(MP(c, m), 0);
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var m: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(m, c);
    v.EB(MP(c, m), 1);
  }
