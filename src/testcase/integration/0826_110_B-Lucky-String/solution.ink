// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  while ((cin >> n))
  {
    {
      var i = 0;
      while ((i < n))
      {
        write(cpp_cast(((cpp_char("a") + (i % 4)))));
        i += 1;
      }
    }
    write("\n");
  }
}
