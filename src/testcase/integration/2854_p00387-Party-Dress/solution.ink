// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  var num = 0;
  while ((b > 0))
  {
    b = (b - a);
    num += 1;
  }
  write(num, "\n");
  return 0;
}
