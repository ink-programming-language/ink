// Translated from solution.cpp.

func main() -> dynamic
{
  var x: dynamic = cpp_array(1000);
  read(x);
  var result: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (strlen(x) + 1)))
    {
      result += 25;
      i += 1;
    }
  }
  write((result + 1));
}
