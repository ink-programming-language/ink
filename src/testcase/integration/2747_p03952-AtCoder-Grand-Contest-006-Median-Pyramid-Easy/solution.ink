// Translated from solution.cpp.

func gm(x: dynamic, n: dynamic) -> dynamic
{
  if ((x > n))
  {
    x -= n;
  }
  if ((x <= 0))
  {
    x += n;
  }
  return x;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n, x);
  var z: dynamic = ((2 * n) - 1);
  if (((x == 1) || (x == z)))
  {
    write("No\n");
    return 0;
  }
  write("Yes\n");
  var st: dynamic = gm(((x - n) + 1), z);
  {
    var i: dynamic = 0;
    while ((i < z))
    {
      write(st, "\n");
      st = gm((st + 1), z);
      i += 1;
    }
  }
}
