// Translated from solution.cpp.

var g: dynamic = 10.0;

var eps: dynamic = 1e-9;

var N: dynamic = (1000 + 10);

var maxn: dynamic = 16;

var inf: dynamic = 9999999;

var n: dynamic = cpp_uninitialized();

func query(x: dynamic, y: dynamic) -> dynamic
{
  if (((x < 0) || (y > n)))
  {
    return 0;
  }
  write(1, " ", x, " ", y, "\n");
  fflush(stdout);
  var s: dynamic = cpp_uninitialized();
  read(s);
  return (s == "TAK");
}

func bir(l: dynamic, r: dynamic) -> dynamic
{
  if ((l > r))
  {
    return -1;
  }
  while ((l < r))
  {
    var m: dynamic = (((l + r)) / 2);
    if (query(m, (m + 1)))
    {
      r = m;
    } else
    {
      l = (m + 1);
    }
  }
  return l;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  x = bir(1, n);
  y = bir(1, (x - 1));
  if ((!query(y, x)))
  {
    y = bir((x + 1), n);
  }
  write(2, " ", x, " ", y, "\n");
  return 0;
}
