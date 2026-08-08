// Translated from solution.cpp.

func main()
{
  var A: dynamic;
  var B: dynamic;
  var C: dynamic;
  var ans = 0;
  var sum = 0;
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
