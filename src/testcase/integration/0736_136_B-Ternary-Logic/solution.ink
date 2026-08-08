// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  var ans = 0;
  var t = 1;
  while ((a || b))
  {
    var x = (a % 3);
    var y = (b % 3);
    var z = ((((y - x) + 3)) % 3);
    ans += (z * t);
    t *= 3;
    a /= 3;
    b /= 3;
  }
  write(ans, "\n");
}
