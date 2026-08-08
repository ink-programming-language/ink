// Translated from solution.cpp.

func main()
{
  var x: dynamic;
  read(x);
  var rjesenja = cpp_array(x);
  {
    var k = 0;
    while ((k < x))
    {
      var n: dynamic;
      var sum = 0;
      read(n);
      var niz = cpp_array(n);
      {
        var i = 0;
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
    var i = 0;
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
