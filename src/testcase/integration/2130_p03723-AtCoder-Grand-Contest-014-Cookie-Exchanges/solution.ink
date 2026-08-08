// Translated from solution.cpp.

func solve(A: dynamic, B: dynamic, C: dynamic)
{
  if ((((A & 1) || (B & 1)) || (C & 1)))
  {
    return 0;
  }
  if (((A == B) && (B == C)))
  {
    return -1;
  }
  return (1 + solve((((B + C)) / 2), (((A + C)) / 2), (((A + B)) / 2)));
}

func main(argument_0: dynamic)
{
  var A: dynamic;
  var B: dynamic;
  var C: dynamic;
  read(A, B, C);
  write(solve(A, B, C), "\n");
  return 0;
}
