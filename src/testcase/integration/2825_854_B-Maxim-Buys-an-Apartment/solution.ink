// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var minimum = 1;
  var maximum: dynamic;
  read(n, k);
  if ((k == n))
  {
    minimum = 0;
  }
  if ((k == 0))
  {
    maximum = 0;
    minimum = 0;
  } else if (((n / k) >= 3))
  {
    maximum = (2 * k);
  } else
  {
    var p = (n / 3);
    maximum = (3 * p);
    k = (k - (n % 3));
    maximum = (maximum - k);
  }
  write(minimum, " ", maximum);
  return 0;
}
