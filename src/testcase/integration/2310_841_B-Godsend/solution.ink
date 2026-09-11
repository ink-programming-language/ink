// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var count: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var elements: dynamic = cpp_uninitialized();
      read(elements);
      if (((elements % 2) != 0))
      {
        count += 1;
      }
      i += 1;
    }
  }
  var res: dynamic =  (((count == 0))) ? "Second" : "First";
  write(res);
  return 0;
}
