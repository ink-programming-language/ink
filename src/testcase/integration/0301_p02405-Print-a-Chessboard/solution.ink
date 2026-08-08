// Translated from solution.cpp.

func main()
{
  var h: dynamic;
  var w: dynamic;
  while (cpp_comma(((cin >> h) >> w), ((h + w) != 0)))
  {
    {
      var i = 0;
      while ((i < h))
      {
        {
          var j = 0;
          while ((j < w))
          {
            write((if ((((((i + j)) % 2) == 0))) "#" else "."));
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
    write("\n");
  }
}
