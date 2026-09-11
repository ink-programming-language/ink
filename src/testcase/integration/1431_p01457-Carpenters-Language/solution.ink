// Translated from solution.cpp.

var typeof: dynamic = cpp_expression("#include");

func builtin_popcount(n: dynamic) -> dynamic
{
  return  (n) ? (1 + builtin_popcount((n & ((n - 1))))) : 0;
}

func foreach(it: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for (__typeof__((c).begin()) it=(c).begin(); it != (c).end(); ++it)");
}

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #inc");
}

func rall(c: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #inclu");
}

func CLEAR(arr: dynamic, val: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio> #include <c");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < n; ++i)");
}

func max_swap(a: dynamic, b: dynamic) -> dynamic
{
  a = max(a, b);
}

func min_swap(a: dynamic, b: dynamic) -> dynamic
{
  a = min(a, b);
}

var EPS: dynamic = 1e-8;

var PI: dynamic = acos(-1.0);

var dx: dynamic = [0, 1, 0, -1];

var dy: dynamic = [1, 0, -1, 0];

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  var par: dynamic = 0;
  while (cpp_update(q, "--"))
  {
    var p: dynamic = cpp_uninitialized();
    var n: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
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
