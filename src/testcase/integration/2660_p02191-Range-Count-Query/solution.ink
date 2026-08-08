// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < (n); ++i)");
}

func srep(i: dynamic, s: dynamic, t: dynamic)
{
  cpp_macro("for (int i = s; i < t; ++i)");
}

func drep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (n)-1; i >= 0; --i)");
}

var yn = cpp_expression("#include <bits");

var MAX_N = cpp_expression("#inclu");

func main()
{
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  var a = cpp_array(n);
  rep(i, n);
  read(a[i]);
  sort(a, (a + n));
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var l: dynamic;
    var r: dynamic;
    read(l, r);
    var ans = (lower_bound(a, (a + n), (r + 1)) - lower_bound(a, (a + n), l));
    write(ans, "\n");
  }
