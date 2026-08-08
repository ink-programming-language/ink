// Translated from solution.cpp.

var s: dynamic;

var v: dynamic;

var arr = cpp_array(int_cpp(1e6));

func main()
{
  var T: dynamic;
  var TT = 1;
  {
    T = 1;
    while ((T <= TT))
    {
      var n: dynamic;
      var i: dynamic;
      var j: dynamic;
      var k = 0;
      var m = 0;
      var g: dynamic;
      var cnt = 1;
      read(n);
      var a = cpp_array(n);
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
