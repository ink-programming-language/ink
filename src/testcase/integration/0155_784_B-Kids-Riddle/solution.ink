// Translated from solution.cpp.

var s = [1, 0, 0, 0, 1, 0, 1, 0, 2, 1, 1, 2, 0, 1, 0, 0];

func main()
{
  var n: dynamic;
  var m = 0;
  scanf("%d", (&n));
  if ((n == 0))
  {
    m = 1;
  }
  while ((n != 0))
  {
    m += s[(n % 16)];
    n /= 16;
  }
  printf("%d", m);
}
