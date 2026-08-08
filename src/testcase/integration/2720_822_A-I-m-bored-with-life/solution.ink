// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var x: dynamic;
  read(a, b);
  x = min(a, b);
  var fact = 1;
  while ((x > 1))
  {
    fact = (fact * x);
    x -= 1;
  }
  write(fact);
  return (0);
}
