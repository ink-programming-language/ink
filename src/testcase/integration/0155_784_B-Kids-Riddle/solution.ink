// Translated from solution.cpp.

var s: dynamic = [1, 0, 0, 0, 1, 0, 1, 0, 2, 1, 1, 2, 0, 1, 0, 0];

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = 0;
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
