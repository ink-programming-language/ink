// Translated from solution.cpp.

var ll = dynamic;

var MOD = cpp_expression("#include <");

var INF = cpp_expression("#inc");

var PI = cpp_expression("#include <bits/s");

func main(argument_0: dynamic)
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var x: dynamic;
  var y: dynamic;
  var xs = 0;
  var ys = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(x);
      xs = (((xs + ((((1 - n) + (2 * i))) * x))) % MOD);
      i += 1;
    }
  }
  {
    var i = 0;
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
