// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0; i<(n); i++)");
}

func FOR(i: dynamic, x: dynamic, n: dynamic)
{
  cpp_macro("for(int i=x; i<(n); i++)");
}

func ALL(n: dynamic)
{
  return cpp_expression("#include <bits/");
}

var MOD = cpp_expression("#include <bi");

var INF = cpp_expression("#incl");

var INFL = cpp_expression("#inclu");

func pr(x: dynamic)
{
  write(x, "\n");
}

func prvec(a: dynamic)
{
  rep(i, (a.size() - 1));
  {
    write(a[i], " ");
  }
  write(a[(a.size() - 1)], "\n");
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

var cub = cpp_array(500, 500, 500);

func solve(n: dynamic, h: dynamic)
{
  cpp_statement("rep(i, n) rep(j, n) rep(k, n)");
  cub[i][j][k] = true;
  var c: dynamic;
  var a: dynamic;
  var b: dynamic;
  var ans = 0;
  rep(i, n);
  rep(j, n);
  rep(k, n) += cub[i][j][k];
  pr(ans);
  return;
}

func main()
{
  var n: dynamic;
  var h: dynamic;
  while (((cin >> n) >> h))
  {
    if (((n == 0) && (h == 0)))
    {
      break;
    }
    solve(n, h);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    read(c, a, b);
    a -= 1;
    b -= 1;
    if ((c == "xy"))
    {
      rep(i, n)[a][b][i] = false;
    } else if ((c == "xz"))
    {
      rep(i, n)[a][i][b] = false;
    } else
    {
      rep(i, n)[i][a][b] = false;
    }
  }
