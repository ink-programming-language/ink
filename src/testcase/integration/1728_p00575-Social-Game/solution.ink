// Translated from solution.cpp.

func main() -> dynamic
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var sum: dynamic = 0;
  read(A, B, C);
  while ((C > sum))
  {
    ans += 1;
    sum += A;
    if (((ans % 7) == 0))
    {
      sum += B;
    }
  }
  write(ans, "\n");
}
