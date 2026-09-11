// Translated from solution.cpp.

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var a: dynamic = cpp_array(n);
  var b: dynamic = cpp_array(n);
  var c: dynamic = cpp_array(m);
  var d: dynamic = cpp_array(m);
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
      var ret: dynamic = 3e8;
      var ans: dynamic = -1;
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
