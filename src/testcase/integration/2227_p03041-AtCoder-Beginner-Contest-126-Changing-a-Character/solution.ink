// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var s = cpp_array(51);
  read(s);
  s[(k - 1)] = ((s[(k - 1)] - cpp_char("A")) + cpp_char("a"));
  write(s, "\n");
}
