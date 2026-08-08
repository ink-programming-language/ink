// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if ((a < 0.001))
  {
    return b;
  } else if ((b < 0.001))
  {
    return a;
  } else
  {
    return gcd(b, fmod(a, b));
  }
}

func main()
{
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  var x3: dynamic;
  var y3: dynamic;
  read(x1, y1, x2, y2, x3, y3);
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  a = sqrt((pow((x1 - x2), 2) + pow((y1 - y2), 2)));
  b = sqrt((pow((x1 - x3), 2) + pow((y1 - y3), 2)));
  c = sqrt((pow((x3 - x2), 2) + pow((y3 - y2), 2)));
  var p: dynamic;
  var r: dynamic;
  var s: dynamic;
  p = ((((a + b) + c)) / 2);
  r = ((((a * b) * c) / 4) / sqrt((((p * ((p - a))) * ((p - b))) * ((p - c)))));
  var A: dynamic;
  var B: dynamic;
  var C: dynamic;
  A = acos((((((((2 * r) * r) - (a * a))) / 2) / r) / r));
  B = acos((((((((2 * r) * r) - (b * b))) / 2) / r) / r));
  C = (((2 * 3.141592653) - A) - B);
  var D = gcd(gcd(A, B), C);
  var k = ((((3.141592653 / D) * r) * r) * sin(D));
  printf("%.8f", k);
}
