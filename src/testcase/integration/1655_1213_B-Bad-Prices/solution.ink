// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var mn: dynamic = cpp_uninitialized();
  read(t);
  {
    i = 0;
    while ((i < t))
    {
      c = 0;
      mn = 999999999;
      read(n);
      var a: dynamic = cpp_array(n);
      {
        j = 0;
        while ((j < n))
        {
          read(a[j]);
          j += 1;
        }
      }
      {
        j = (n - 1);
        while ((j >= 0))
        {
          if ((mn >= a[j]))
          {
            mn = a[j];
          } else
          {
            c += 1;
          }
          j -= 1;
        }
      }
      write(c, "\n");
      i += 1;
    }
  }
}
