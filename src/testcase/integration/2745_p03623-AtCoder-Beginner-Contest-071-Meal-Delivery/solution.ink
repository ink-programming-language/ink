// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var x: dynamic;
  read(x, a, b);
  if ((abs((x - a)) < abs((x - b))))
  {
    write(cpp_char("A"), "\n");
  } else
  {
    write(cpp_char("B"), "\n");
  }
  return 0;
}
