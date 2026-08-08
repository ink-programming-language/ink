// Translated from solution.cpp.

func clr(x: dynamic)
{
  return cpp_expression("#include <bits/stdc+");
}

func For(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i=(a);i<=(b);i++)");
}

func Fod(i: dynamic, b: dynamic, a: dynamic)
{
  cpp_macro("for (int i=(b);i>=(a);i--)");
}

func pb(x: dynamic)
{
  return cpp_expression("#include <bi");
}

func mp(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <bits");
}

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func outval(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func outtag(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> #define clr(x) memset(x");
}

func outarr(a: dynamic, L: dynamic, R: dynamic)
{
  cpp_macro("cerr<<#a\"[\"<<L<<\"..\"<<R<<\"] = \";\\\n                    For(_x,L,R) cerr<<a[_x]<<\" \";cerr<<endl;");
}

func read()
{
  var x = 0;
  var f = 0;
  var ch = getchar();
  while ((!isdigit(ch)))
  {
    f = (ch == cpp_char("-"));
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = ((((x << 1)) + ((x << 3))) + ((ch ^ 48)));
    ch = getchar();
  }
  return if (f) (-x) else x;
}

var N = 105;

var n: dynamic;

var m: dynamic;

var a = cpp_array(N, N);

var b = cpp_array(N);

var g = cpp_array(N, N);

var vis = cpp_array(N);

var Match = cpp_array(N);

func dfs(x: dynamic)
{
  cpp_statement("For(y,1,n)");
  if ((g[x][y] && (!vis[y])))
  {
    vis[y] = 1;
    if (((!Match[y]) || dfs(Match[y])))
    {
      Match[y] = x;
      return 1;
    }
  }
  return 0;
}

func main()
{
  n = read();
  m = read();
  For(i, 1, n);
  For(j, 1, m)[i][j] = read();
  For(c, 1, m);
  {
    clr(g);
    For(i, 1, n);
    clr(Match);
    For(i, 1, n);
    {
      clr(vis);
      assert(dfs(i));
    }
    For(i, 1, n);
    {
      var r = Match[i];
      swap(a[r][c], a[r][g[r][i]]);
    }
  }
  For(i, 1, n);
  {
    cpp_statement("For(j,1,m)");
    printf("%d ", a[i][j]);
    puts("");
  }
  For(i, 1, m);
  {
    For(j, 1, n)[j] = a[j][i];
    sort((b + 1), ((b + n) + 1));
    For(j, 1, n)[j][i] = b[j];
  }
  For(i, 1, n);
  {
    cpp_statement("For(j,1,m)");
    printf("%d ", a[i][j]);
    puts("");
  }
  return 0;
}

func For(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic)
{
      var r = ((((a[i][j] + m) - 1)) / m);
      if ((!g[i][r]))
      {
        g[i][r] = j;
      }
    }
