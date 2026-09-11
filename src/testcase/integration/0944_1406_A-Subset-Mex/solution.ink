// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var a: dynamic = cpp_array(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort(a, (a + n));
    var m: dynamic = 0;
    var k: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((a[i] == m))
        {
          m += 1;
        } else if ((a[i] == k))
        {
          k += 1;
        }
        i += 1;
      }
    }
    write((m + k), "\n");
  }
  return 0;
}
