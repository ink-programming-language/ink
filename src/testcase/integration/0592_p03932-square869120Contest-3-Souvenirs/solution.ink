// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (ll i = (signed)(a); i < (b); ++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <bi");
}

func EREP(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (n)-1; i >= 0; --i)");
}

var MOD = cpp_expression("#include <");

var pb = cpp_expression("#include");

var INF = cpp_expression("#include <bits/st");

var MIN = cpp_expression("#include <bits/std");

var EPS = cpp_expression("#incl");

func lb(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> typedef long lo");
}

func ub(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> typedef long lo");
}

func bitcnt(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> typ");
}

func fill_all(arr: dynamic, v: dynamic)
{
  arr = v;
}

func fill_all(arr: dynamic, v: dynamic)
{
  for (var i in arr)
  {
    fill_all(i, v);
  }
}

var h: dynamic;

var w: dynamic;

var grid = cpp_array(200, 200);

var dp = cpp_array(200, 200, 500);

var p = [1, 0, 0, 1, 1, 1, 0, 0];

func main()
{
  read(h, w);
  REP(i, ((h + w) - 2));
  {
    REP(X, min(w, (i + 1)));
    {
      REP(x, min(w, (i + 1)));
      {
        if (((h <= (i - X)) || (h <= (i - x))))
        {
          continue;
        }
        var y = (i - x);
        var Y = (i - X);
        var cost = (grid[Y][X] + grid[y][x]);
        cost -= (if ((X == x)) grid[y][x] else 0);
        REP(k, 4);
        {
          dp[(i + 1)][(X + p[(k * 2)])][(x + p[((k * 2) + 1)])] = max(dp[(i + 1)][(X + p[(k * 2)])][(x + p[((k * 2) + 1)])], (cost + dp[i][X][x]));
        }
      }
    }
  }
  write((dp[((h + w) - 2)][(w - 1)][(w - 1)] + grid[(h - 1)][(w - 1)]), "\n");
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      read(grid[i][j]);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
  }
