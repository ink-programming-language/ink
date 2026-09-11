// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> n), n))
  {
    var cnt: dynamic = 0;
    var ok: dynamic = false;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var k: dynamic = cpp_uninitialized();
        read(k);
        if ((k > 0))
        {
          cnt += 1;
        }
        if ((k > 1))
        {
          ok = true;
        }
        i += 1;
      }
    }
    if (ok)
    {
      write((cnt + 1), "\n");
    } else
    {
      write("NA", "\n");
    }
  }
  return 0;
}
