// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var mini: dynamic = 33;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      mini = min(mini, a[i]);
      i += 1;
    }
  }
  write((2 + ((a[2] ^ mini))), "\n");
  return 0;
}
