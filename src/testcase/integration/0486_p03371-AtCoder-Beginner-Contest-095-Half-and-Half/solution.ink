// Translated from solution.cpp.

func main() -> dynamic
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  var Y: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
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
