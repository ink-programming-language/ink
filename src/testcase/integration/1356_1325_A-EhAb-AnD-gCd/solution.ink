// Translated from solution.cpp.

func valid(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  return ((x + y) > z);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    write(1, " ", (n - 1), "\n");
  }
}
