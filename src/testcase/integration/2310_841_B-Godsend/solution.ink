// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var count = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var elements: dynamic;
      read(elements);
      if (((elements % 2) != 0))
      {
        count += 1;
      }
      i += 1;
    }
  }
  var res = if (((count == 0))) "Second" else "First";
  write(res);
  return 0;
}
