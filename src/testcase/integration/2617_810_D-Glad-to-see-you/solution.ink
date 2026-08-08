// Translated from solution.cpp.

var g = 10.0;

var eps = 1e-9;

var N = (1000 + 10);

var maxn = 16;

var inf = 9999999;

var n: dynamic;

func query(x: dynamic, y: dynamic)
{
  if (((x < 0) || (y > n)))
  {
    return 0;
  }
  write(1, " ", x, " ", y, "\n");
  fflush(stdout);
  var s: dynamic;
  read(s);
  return (s == "TAK");
}

func bir(l: dynamic, r: dynamic)
{
  if ((l > r))
  {
    return -1;
  }
  while ((l < r))
  {
    var m = (((l + r)) / 2);
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var x: dynamic;
  var y: dynamic;
  var k: dynamic;
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
