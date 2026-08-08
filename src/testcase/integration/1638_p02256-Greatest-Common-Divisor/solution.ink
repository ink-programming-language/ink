// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  write(gcd(a, b), "\n");
}
