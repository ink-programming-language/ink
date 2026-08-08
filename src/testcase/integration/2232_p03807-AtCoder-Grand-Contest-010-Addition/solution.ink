// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var k = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      read(a);
      if ((a % 2))
      {
        k += 1;
      }
      i += 1;
    }
  }
  if ((k % 2))
  {
    write("NO", "\n");
  } else
  {
    write("YES", "\n");
  }
}
