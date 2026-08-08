// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var d: dynamic;
  read(n, d);
  {
    var i = 1;
    while ((i < n))
    {
      var a: dynamic;
      read(a);
      if ((i != ((((a + (d * (if ((i & 1)) 1 else -1))) + n)) % n)))
      {
        write("No", "\n");
        return 0;
      }
      i += 1;
    }
  }
  write("Yes", "\n");
}
