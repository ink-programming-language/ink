// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < (n); ++i)");
}

func srep(i: dynamic, s: dynamic, t: dynamic) -> dynamic
{
  cpp_macro("for (int i = s; i < t; ++i)");
}

func drep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = (n)-1; i >= 0; --i)");
}

var yn: dynamic = cpp_expression("#include <bits");

var MAX_N: dynamic = cpp_expression("#inclu");

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  var a: dynamic = cpp_array(n);
  rep(i, n);
  read(a[i]);
  sort(a, (a + n));
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    read(l, r);
    var ans: dynamic = (lower_bound(a, (a + n), (r + 1)) - lower_bound(a, (a + n), l));
    write(ans, "\n");
  }
