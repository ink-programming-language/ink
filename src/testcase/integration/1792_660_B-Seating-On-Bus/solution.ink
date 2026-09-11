// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  var val: dynamic = 1;
  {
    var i: dynamic = 0;
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
