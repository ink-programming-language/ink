// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((cin >> n))
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        write(cpp_cast(((cpp_char("a") + (i % 4)))));
        i += 1;
      }
    }
    write("\n");
  }
}
