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
    var i: dynamic;
    {
      i = 0;
      while ((i < (n - 1)))
      {
        if (((a[(i + 1)] - a[i]) > 1))
        {
          write("NO", "\n");
          break;
        }
        i += 1;
      }
    }
    if ((i == (n - 1)))
    {
      write("YES", "\n");
    }
  }
}
