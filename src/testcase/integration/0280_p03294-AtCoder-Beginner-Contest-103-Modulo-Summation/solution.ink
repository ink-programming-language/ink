// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(n);
  {
    var i: dynamic = 0;
    var a: dynamic = cpp_uninitialized();
    while ((i < n))
    {
      read(a);
      ans += (a - 1);
      i += 1;
    }
  }
  write(ans, "\n");
}
