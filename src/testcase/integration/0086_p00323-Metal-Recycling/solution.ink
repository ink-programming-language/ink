// Translated from solution.cpp.

var INT_MAX = cpp_expression("#include <");

var INF = cpp_expression("#include <");

var MOD = cpp_expression("#include <");

var ll = dynamic;

func rep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(i = (a); i < (b); i++)");
}

func bitget(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <iostream");
}

var i: dynamic;

var j: dynamic;

var k: dynamic;

func main()
{
  var n: dynamic;
  var a = cpp_array(100000);
  var b = cpp_array(100000);
  read(n);
  rep(i, 0, n);
  read(a[i], b[i]);
  var c = [];
  var d = [];
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
