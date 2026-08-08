// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    var cnt = 0;
    var ok = false;
    {
      var i = 0;
      while ((i < n))
      {
        var k: dynamic;
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
