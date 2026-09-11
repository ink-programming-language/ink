// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var p: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(p[i]);
      i += 1;
    }
  }
  sort(p, (p + n));
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      sum += p[i];
      i += 1;
    }
  }
  write(sum, "\n");
}
