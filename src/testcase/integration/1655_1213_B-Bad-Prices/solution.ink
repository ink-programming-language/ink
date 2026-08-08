// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var c: dynamic;
  var mn: dynamic;
  read(t);
  {
    i = 0;
    while ((i < t))
    {
      c = 0;
      mn = 999999999;
      read(n);
      var a = cpp_array(n);
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
