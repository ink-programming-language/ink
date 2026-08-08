// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  {
    var i = 0;
    while ((i < t))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      if (((a % b) == 0))
      {
        write("0\n");
      } else
      {
        write((b - ((a % b))), "\n");
      }
      i += 1;
    }
  }
}
