// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
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
  var d = (b - a);
  var k = (d / c);
  if ((((d % c) != 0) || (k < 0)))
  {
    write("NO", cpp_char("\n"));
    return 0;
  }
  write("YES", cpp_char("\n"));
  return 0;
}
