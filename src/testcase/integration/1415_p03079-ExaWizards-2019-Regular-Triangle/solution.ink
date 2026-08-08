// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  read(a, b, c);
  if (cpp_binary((a == b), "and", (b == c)))
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
  return 0;
}
