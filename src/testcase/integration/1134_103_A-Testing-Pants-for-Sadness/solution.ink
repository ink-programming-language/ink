// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var res: dynamic = 0;
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(t);
      res += (((t * i)) - ((i - 1)));
      i += 1;
    }
  }
  write(res, "\n");
}
