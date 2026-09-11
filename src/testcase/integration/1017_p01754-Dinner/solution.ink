// Translated from solution.cpp.

func syosu(x: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #i");
}

var inf: dynamic = (1 << 28);

var INF: dynamic = (1 << 60);

var pi: dynamic = acos(-1);

var eps: dynamic = 1e-8;

var mod: dynamic = (1e9 + 7);

var dx: dynamic = [0, 1, 0, -1];

var dy: dynamic = [1, 0, -1, 0];

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, p, q);
  a = vl(n);
  var sum: dynamic = 0;
  var res: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      sum += a[i];
      a[i] = ((p * ((q - i))) - a[i]);
      i += 1;
    }
  }
  sort(a.rbegin(), a.rend());
  res = sum;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      sum += a[i];
      res = max(res, (sum + ((p * i) * ((i + 1)))));
      i += 1;
    }
  }
  write(res, "\n");
}
