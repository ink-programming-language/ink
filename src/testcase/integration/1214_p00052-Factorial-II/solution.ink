// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var n: dynamic;
  var c: dynamic;
  while (true)
  {
    read(n);
    if ((n == 0))
    {
      break;
    }
    c = 0;
    {
      while ((0 < n))
      {
        n /= 5;
        c += n;
      }
    }
    write(c, "\n");
  }
}
