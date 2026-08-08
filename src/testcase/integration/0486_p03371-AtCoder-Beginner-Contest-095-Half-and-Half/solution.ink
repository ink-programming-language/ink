// Translated from solution.cpp.

func main()
{
  var A: dynamic;
  var B: dynamic;
  var C: dynamic;
  var X: dynamic;
  var Y: dynamic;
  var ans = 0;
  read(A, B, C, X, Y);
  if (((A + B) < (2 * C)))
  {
    ans = ((X * A) + (Y * B));
  } else
  {
    ans = ((((min(X, Y) * C) * 2) + (max((X - Y), 0) * min(A, (2 * C)))) + (max((Y - X), 0) * min(B, (2 * C))));
  }
  write(ans, "\n");
}
