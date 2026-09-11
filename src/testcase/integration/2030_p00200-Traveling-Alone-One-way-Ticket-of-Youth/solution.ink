// Translated from solution.cpp.

func REP(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for (int i = (a); i < (b); ++i)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <cs");
}

var INF: dynamic = cpp_expression("#includ");

var C: dynamic = cpp_array(100, 100, 2);

func main() -> dynamic
{
  while (true)
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    scanf("%d %d", (&n), (&m));
    if (((n == 0) && (m == 0)))
    {
      break;
    }
    rep(k, 2);
    rep(i, 100);
    {
      rep(j, 100);
      {
        C[k][i][j] = INF;
      }
      C[k][i][i] = 0;
    }
    rep(k, m);
    rep(i, m);
    rep(j, m);
    rep(l, 2);
    {
      C[l][i][j] = min((C[l][i][k] + C[l][k][j]), C[l][i][j]);
    }
    var k: dynamic = cpp_uninitialized();
    scanf("%d", (&k));
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf("%d%d%d%d", (&a), (&b), (&c), (&t));
      a -= 1;
      b -= 1;
      C[0][a][b] = cpp_assign(C[0][b][a], "=", c);
      C[1][a][b] = cpp_assign(C[1][b][a], "=", t);
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var p: dynamic = cpp_uninitialized();
      var q: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&p), (&q), (&r));
      printf("%d\n", C[r][(p - 1)][(q - 1)]);
    }
