// Translated from solution.cpp.

var typeof = cpp_expression("#include");

func builtin_popcount(n: dynamic)
{
  return if (n) (1 + builtin_popcount((n & ((n - 1))))) else 0;
}

func foreach(it: dynamic, c: dynamic)
{
  cpp_macro("for (__typeof__((c).begin()) it=(c).begin(); it != (c).end(); ++it)");
}

func all(c: dynamic)
{
  return cpp_expression("#include <cstdio> #inc");
}

func rall(c: dynamic)
{
  return cpp_expression("#include <cstdio> #inclu");
}

func CLEAR(arr: dynamic, val: dynamic)
{
  return cpp_expression("#include <cstdio> #include <c");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < n; ++i)");
}

func max_swap(a: dynamic, b: dynamic)
{
  a = max(a, b);
}

func min_swap(a: dynamic, b: dynamic)
{
  a = min(a, b);
}

var EPS = 1e-8;

var PI = acos(-1.0);

var dx = [0, 1, 0, -1];

var dy = [1, 0, -1, 0];

func main()
{
  var q: dynamic;
  scanf("%d", (&q));
  var par = 0;
  while (cpp_update(q, "--"))
  {
    var p: dynamic;
    var n: dynamic;
    var c: dynamic;
    scanf("%d %c %d", (&p), (&c), (&n));
    if ((c == cpp_char("(")))
    {
      par += n;
    } else
    {
      par -= n;
    }
    if ((par == 0))
    {
      puts("Yes");
    } else
    {
      puts("No");
    }
  }
}
