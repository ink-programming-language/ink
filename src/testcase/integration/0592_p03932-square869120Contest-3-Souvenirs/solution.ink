// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for (ll i = (signed)(a); i < (b); ++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <bi");
}

func EREP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (n)-1; i >= 0; --i)");
}

var MOD: dynamic = cpp_expression("#include <");

var pb: dynamic = cpp_expression("#include");

var INF: dynamic = cpp_expression("#include <bits/st");

var MIN: dynamic = cpp_expression("#include <bits/std");

var EPS: dynamic = cpp_expression("#incl");

func lb(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> typedef long lo");
}

func ub(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> typedef long lo");
}

func bitcnt(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> typ");
}

func fill_all(arr: dynamic, v: dynamic) -> dynamic
{
  arr = v;
}

func fill_all(arr: dynamic, v: dynamic) -> dynamic
{
  for (var i: dynamic in arr)
  {
    fill_all(i, v);
  }
}

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var grid: dynamic = cpp_array(200, 200);

var dp: dynamic = cpp_array(200, 200, 500);

var p: dynamic = [1, 0, 0, 1, 1, 1, 0, 0];

func main() -> dynamic
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
        var y: dynamic = (i - x);
        var Y: dynamic = (i - X);
        var cost: dynamic = (grid[Y][X] + grid[y][x]);
        cost -= ( ((X == x)) ? grid[y][x] : 0);
        REP(k, 4);
        {
          dp[(i + 1)][(X + p[(k * 2)])][(x + p[((k * 2) + 1)])] = max(dp[(i + 1)][(X + p[(k * 2)])][(x + p[((k * 2) + 1)])], (cost + dp[i][X][x]));
        }
      }
    }
  }
  write((dp[((h + w) - 2)][(w - 1)][(w - 1)] + grid[(h - 1)][(w - 1)]), "\n");
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(grid[i][j]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
  }
