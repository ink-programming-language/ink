// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var i = 0;
  var s: dynamic;
  read(N, s);
  {
    i = 1;
    while ((i < N))
    {
      if (((s[i] == cpp_char("x")) && (s[(i - 1)] == cpp_char("x"))))
      {
        break;
      }
      i += 1;
    }
  }
  write(i, "\n");
  return 0;
}
