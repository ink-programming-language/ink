// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var j: dynamic = 0;
      while (((i + ((1 << ((j + 1))))) < n))
      {
        j += 1;
      }
      a[(i + ((1 << j)))] += a[i];
      ans += a[i];
      write(ans, cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
