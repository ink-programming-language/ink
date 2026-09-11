// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var r: dynamic = cpp_uninitialized();
  read(r);
  if (((r % 2) == 0))
  {
    write("NO");
    return 0;
  }
  if ((r < 4))
  {
    write("NO");
    return 0;
  }
  r -= 1;
  r /= 2;
  write("1 ", (r - 1));
  return 0;
}
