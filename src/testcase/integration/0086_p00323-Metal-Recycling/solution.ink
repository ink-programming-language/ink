// Translated from solution.cpp.

var INT_MAX: dynamic = cpp_expression("#include <");

var INF: dynamic = cpp_expression("#include <");

var MOD: dynamic = cpp_expression("#include <");

var ll: dynamic = dynamic;

func rep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(i = (a); i < (b); i++)");
}

func bitget(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream");
}

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(100000);
  var b: dynamic = cpp_array(100000);
  read(n);
  rep(i, 0, n);
  read(a[i], b[i]);
  var c: dynamic = [];
  var d: dynamic = [];
  rep(i, 0, n)[i] = (a[i] + b[i]);
  sort(d, (d + n));
  rep(i, 0, n);
  {
    c[(d[i] / 31)] += ((1 << ((d[i] % 31))));
    if ((c[(d[i] / 31)] < 0))
    {
      c[(d[i] / 31)] = (c[(d[i] / 31)] & 2147483647);
      c[((d[i] / 31) + 1)] += 1;
    }
  }
  rep(i, 0, 1000000);
  {
    if ((bitget(c[(i / 31)], (i % 31)) == 1))
    {
      write(i, " ", "0", "\n");
    }
  }
  return 0;
}
