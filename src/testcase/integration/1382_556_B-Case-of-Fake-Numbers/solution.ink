// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, d);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      read(a);
      if ((i != ((((a + (d * ( ((i & 1)) ? 1 : -1))) + n)) % n)))
      {
        write("No", "\n");
        return 0;
      }
      i += 1;
    }
  }
  write("Yes", "\n");
}
