// Translated from solution.cpp.

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  read(x);
  var rjesenja: dynamic = cpp_array(x);
  {
    var k: dynamic = 0;
    while ((k < x))
    {
      var n: dynamic = cpp_uninitialized();
      var sum: dynamic = 0;
      read(n);
      var niz: dynamic = cpp_array(n);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          read(niz[i]);
          if ((niz[i] <= 2048))
          {
            sum += niz[i];
          }
          i += 1;
        }
      }
      if ((sum >= 2048))
      {
        rjesenja[k] = true;
      } else
      {
        rjesenja[k] = false;
      }
      k += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < x))
    {
      if ((rjesenja[i] == true))
      {
        write("YES", "\n");
      } else
      {
        write("NO", "\n");
      }
      i += 1;
    }
  }
  return 0;
}
