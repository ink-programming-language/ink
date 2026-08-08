// Translated from solution.cpp.

func gcd(x: dynamic, y: dynamic)
{
  while (((cpp_assign(x, "%=", y)) && (cpp_assign(y, "%=", x))))
  {
  }
  return (x ^ y);
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var T: dynamic;
  read(T);
  while (cpp_update(T, "--"))
  {
    var A: dynamic;
    var B: dynamic;
    var C: dynamic;
    var D: dynamic;
    read(A, B, C, D);
    if (((A < B) || (D < B)))
    {
      write("No\n");
      continue;
    }
    var g = gcd(B, D);
    A = (A - (((C - B) + 1)));
    var d = ((((A % g) + g)) % g);
    A = (((C - B) + 1) + d);
    if ((A < 0))
    {
      write("No\n");
    } else
    {
      write("Yes\n");
    }
  }
}
