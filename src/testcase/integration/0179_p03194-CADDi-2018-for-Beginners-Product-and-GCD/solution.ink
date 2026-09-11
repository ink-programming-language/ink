// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var P: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var ans: dynamic = 1;
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
