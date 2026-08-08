// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var k1: dynamic;
  read(a, b);
  k1 = (a % b);
  write(min(k1, (b - k1)), "\n");
}
