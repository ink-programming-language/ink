// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(int_cpp(1e6));

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  var TT: dynamic = 1;
  {
    T = 1;
    while ((T <= TT))
    {
      var n: dynamic = cpp_uninitialized();
      var i: dynamic = cpp_uninitialized();
      var j: dynamic = cpp_uninitialized();
      var k: dynamic = 0;
      var m: dynamic = 0;
      var g: dynamic = cpp_uninitialized();
      var cnt: dynamic = 1;
      read(n);
      var a: dynamic = cpp_array(n);
      {
        i = 0;
        while ((i < n))
        {
          read(a[i]);
          a[i] = abs(a[i]);
          i += 1;
        }
      }
      sort(a, (a + n));
      i = 0;
      j = 1;
      while (((i < n) && (j < n)))
      {
        if (((max(abs((a[i] + a[j])), abs((a[i] - a[j]))) >= max(abs(a[i]), abs(a[j]))) && (min(abs((a[i] + a[j])), abs((a[i] - a[j]))) <= min(abs(a[i]), abs(a[j])))))
        {
          k += ((j - i));
          j += 1;
        } else
        {
          m = i;
          if ((i == (j - 1)))
          {
            i += 1;
            j += 1;
          } else
          {
            i += 1;
          }
        }
      }
      write(k, "\n");
      T += 1;
    }
  }
  return 0;
}
