// Translated from solution.cpp.

func valid(x: dynamic, y: dynamic, z: dynamic)
{
  return ((x + y) > z);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    write(1, " ", (n - 1), "\n");
  }
}
