// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  scanf("%d%d", (&n), (&m));
  var val = 1;
  {
    var i = 0;
    while ((i < n))
    {
      if (((val + (2 * n)) <= m))
      {
        write((val + (2 * n)), " ");
      }
      if ((val <= m))
      {
        write(val, " ");
      }
      if ((((val + (2 * n)) + 1) <= m))
      {
        write(((val + (2 * n)) + 1), " ");
      }
      if (((val + 1) <= m))
      {
        write((val + 1), " ");
      }
      val = (val + 2);
      i += 1;
    }
  }
  return 0;
}
