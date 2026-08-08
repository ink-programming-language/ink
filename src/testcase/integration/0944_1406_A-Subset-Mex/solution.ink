// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var a = cpp_array(n);
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort(a, (a + n));
    var m = 0;
    var k = 0;
    {
      var i = 0;
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
