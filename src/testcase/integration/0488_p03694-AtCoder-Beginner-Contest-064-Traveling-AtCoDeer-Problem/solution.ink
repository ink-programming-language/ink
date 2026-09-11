// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_array(1000);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(k[i]);
      i += 1;
    }
  }
  sort(k, (k + n));
  write((k[(n - 1)] - k[0]), "\n");
}
