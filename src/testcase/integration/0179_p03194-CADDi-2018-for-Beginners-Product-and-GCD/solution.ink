// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var N: dynamic;
  var P: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var ans = 1;
  read(N, P);
  if ((N == 1))
  {
    write(P, "\n");
    return 0;
  }
  {
    i = 2;
    while (((P > 1) && ((i * i) < (P + 1))))
    {
      k = 0;
      while (((P % i) == 0))
      {
        k += 1;
        P /= i;
      }
      {
        j = 0;
        while ((j < (k / N)))
        {
          ans *= i;
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
