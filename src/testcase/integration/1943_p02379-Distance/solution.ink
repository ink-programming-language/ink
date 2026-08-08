// Translated from solution.cpp.

func main()
{
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  var ans: dynamic;
  read(x1, y1, x2, y2);
  ans = sqrt(((((x2 - x1)) * ((x2 - x1))) + (((y2 - y1)) * ((y2 - y1)))));
  write(fixed, ans, "\n");
}
