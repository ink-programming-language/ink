// Translated from solution.cpp.

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> h) >> w), ((h + w) != 0)))
  {
    {
      var i: dynamic = 0;
      while ((i < h))
      {
        {
          var j: dynamic = 0;
          while ((j < w))
          {
            write(( ((((((i + j)) % 2) == 0))) ? "#" : "."));
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
