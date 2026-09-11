// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var sum: dynamic = 0;
    var j: dynamic = (((n - 1)) / 2);
    {
      var i: dynamic = 0;
      while ((i < j))
      {
        sum += (pow((i + 1), 2) * 8);
        i += 1;
      }
    }
    write(sum, "\n");
  }
}
