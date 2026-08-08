// Translated from solution.cpp.

func main()
{
  var i: dynamic;
  var j: dynamic;
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a = cpp_array(n);
  var b = cpp_array(n);
  var c = cpp_array(m);
  var d = cpp_array(m);
  {
    i = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < m))
    {
      read(c[i], d[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      var ret = 3e8;
      var ans = -1;
      {
        j = 0;
        while ((j < m))
        {
          if ((ret > ((abs((a[i] - c[j])) + abs((b[i] - d[j]))))))
          {
            ret = (abs((a[i] - c[j])) + abs((b[i] - d[j])));
            ans = (j + 1);
          }
          j += 1;
        }
      }
      write(ans, "\n");
      i += 1;
    }
  }
  return 0;
}
