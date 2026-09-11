// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  write(a[(((n - 1)) / 2)]);
  return 0;
}
