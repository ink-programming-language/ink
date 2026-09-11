// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(a, b, c);
  if ((c == 0))
  {
    if ((a == b))
    {
      write("YES", cpp_char("\n"));
    } else
    {
      write("NO", cpp_char("\n"));
    }
    return 0;
  }
  var d: dynamic = (b - a);
  var k: dynamic = (d / c);
  if ((((d % c) != 0) || (k < 0)))
  {
    write("NO", cpp_char("\n"));
    return 0;
  }
  write("YES", cpp_char("\n"));
  return 0;
}
