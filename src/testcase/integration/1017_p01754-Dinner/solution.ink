// Translated from solution.cpp.

func syosu(x: dynamic)
{
  return cpp_expression("#include <iostream> #i");
}

var inf = (1 << 28);

var INF = (1 << 60);

var pi = acos(-1);

var eps = 1e-8;

var mod = (1e9 + 7);

var dx = [0, 1, 0, -1];

var dy = [1, 0, -1, 0];

var n: dynamic;

var p: dynamic;

var q: dynamic;

var a: dynamic;

func main()
{
  read(n, p, q);
  a = vl(n);
  var sum = 0;
  var res: dynamic;
  {
    var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      sum += a[i];
      res = max(res, (sum + ((p * i) * ((i + 1)))));
      i += 1;
    }
  }
  write(res, "\n");
}
