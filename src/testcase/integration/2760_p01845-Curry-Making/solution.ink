// Translated from solution.cpp.

func main()
{
  var R0: dynamic;
  var W0: dynamic;
  var C: dynamic;
  var R: dynamic;
  while (((((cin >> R0) >> W0) >> C) >> R))
  {
    if (((((R0 == 0) && (W0 == 0)) && (C == 0)) && (R == 0)))
    {
      return 0;
    }
    var LOSS = ((W0 * C) - R0);
    if ((LOSS <= 0))
    {
      write(0, "\n");
    } else if (((LOSS % R) == 0))
    {
      write((LOSS / R), "\n");
    } else
    {
      write(((LOSS / R) + 1), "\n");
    }
  }
}
