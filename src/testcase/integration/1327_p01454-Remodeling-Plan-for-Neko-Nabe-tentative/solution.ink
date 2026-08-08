// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(ll (i) = (0);(i) < (n);++i)");
}

func REV(i: dynamic, n: dynamic)
{
  cpp_macro("for(ll (i) = (n) - 1;(i) >= 0;--i)");
}

var PB = cpp_expression("#include");

var EB = cpp_expression("#include <bi");

var MP = cpp_expression("#include");

var FI = cpp_expression("#incl");

var SE = cpp_expression("#inclu");

func SHOW1d(v: dynamic, n: dynamic)
{
  cpp_macro("{REP(WW,n)cerr << v[WW] << ' ';cerr << endl << endl;}");
}

func SHOW2d(v: dynamic, WW: dynamic, HH: dynamic)
{
  cpp_macro("{REP(W_,WW){REP(H_,HH)cerr << v[W_][H_] << ' ';cerr << endl;}cerr << endl;}");
}

func ALL(v: dynamic)
{
  return cpp_expression("#include <bits/st");
}

var Decimal = cpp_expression("#include <bits/stdc++.h");

var INF = cpp_expression("#include <");

var LLINF = cpp_expression("#include <bits/stdc++");

var MOD = cpp_expression("#include");

var ans = LLINF;

var dp = cpp_array(11111, 2);

func init()
{
  cpp_statement("REP(i, 2)REP(j, 11111)dp[i][j] = -LLINF; REP(i, 2)");
  dp[i][0] = LLINF;
}

func check(c_max: dynamic, a: dynamic, b: dynamic, w: dynamic)
{
  var deq: dynamic;
  var r = 1;
  {
    var i = 1;
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

func query(cat: dynamic, type_cpp: dynamic, w: dynamic)
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

func main()
{
  cin.tie(0);
  cout.tie(0);
  ios.sync_with_stdio(false);
  init();
  var na: dynamic;
  var nb: dynamic;
  var w: dynamic;
  read(na, nb, w);
  var v: dynamic;
  sort(ALL(v));
  REP(i, v.size());
  {
    query(v[i].FI, v[i].SE, w);
  }
  write(ans, "\n");
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var m: dynamic;
    var c: dynamic;
    read(m, c);
    v.EB(MP(c, m), 0);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var m: dynamic;
    var c: dynamic;
    read(m, c);
    v.EB(MP(c, m), 1);
  }
