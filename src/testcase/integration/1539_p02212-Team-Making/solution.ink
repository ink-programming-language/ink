// Translated from solution.cpp.

func main()
{
  var A: dynamic;
  var B: dynamic;
  var C: dynamic;
  var D: dynamic;
  read(A, B, C, D);
  write(min(min(abs((((A + B) - C) - D)), abs((((A + C) - B) - D))), abs((((A + D) - B) - C))), "\n");
}
