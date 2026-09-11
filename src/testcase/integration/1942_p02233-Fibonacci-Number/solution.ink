// Translated from solution.cpp.

func main() -> dynamic
{
  var MAX: dynamic = 46;
  var f: dynamic = [1, 1];
  {
    var i: dynamic = 2;
    while ((i < MAX))
    {
      f[i] = (f[(i - 1)] + f[(i - 2)]);
      i += 1;
    }
  }
  var n: dynamic = cpp_uninitialized();
  read(n);
  write(f[n], "\n");
  return 0;
}
