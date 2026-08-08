// Translated from solution.cpp.

func main()
{
  var k: dynamic;
  var a: dynamic;
  var b: dynamic;
  read(k, a, b);
  var g1 = (a / k);
  var g2 = (b / k);
  var mod2 = (b % k);
  var mod1 = (a % k);
  if (((mod1 != 0) && (g2 == 0)))
  {
    write("-1");
    return 0;
  }
  if (((mod2 != 0) && (g1 == 0)))
  {
    write("-1");
    return 0;
  }
  write((g1 + g2));
  return 0;
}
