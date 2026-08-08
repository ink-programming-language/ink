// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a = cpp_array(100);
  var b = cpp_array(100);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  {
    var i = 0;
    while ((i < m))
    {
      read(b[i]);
      i += 1;
    }
  }
  sort(b, (b + m));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          if ((a[i] == b[j]))
          {
            write(a[i], cpp_char("\n"));
            return 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((a[0] > b[0]))
  {
    swap(a[0], b[0]);
  }
  write(a[0], b[0], cpp_char("\n"));
}
