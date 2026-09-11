// Translated from solution.cpp.

var ll: dynamic = dynamic;

var MOD: dynamic = cpp_expression("#include <");

var INF: dynamic = cpp_expression("#inc");

var PI: dynamic = cpp_expression("#include <bits/s");

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var xs: dynamic = 0;
  var ys: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      xs = (((xs + ((((1 - n) + (2 * i))) * x))) % MOD);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(y);
      ys = (((ys + ((((1 - m) + (2 * i))) * y))) % MOD);
      i += 1;
    }
  }
  write(((xs * ys) % MOD), "\n");
  return 0;
}
