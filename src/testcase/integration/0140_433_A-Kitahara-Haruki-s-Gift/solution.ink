// Translated from solution.cpp.

var max1 = 100010;

var a = cpp_array(max1);

var b = cpp_array(max1);

func main()
{
  var n: dynamic;
  var x: dynamic;
  var y = 0;
  var z = 0;
  read(n);
  if ((n == 1))
  {
    write("NO\n");
  } else
  {
    {
      var i = 0;
      while ((i < n))
      {
        read(x);
        if ((x == 200))
        {
          z += 1;
        }
        y += x;
        i += 1;
      }
    }
    if ((((y % 200) == 0)))
    {
      if ((((z == n)) && (((z % 2) != 0))))
      {
        write("NO\n");
      } else
      {
        write("YES\n");
      }
    } else
    {
      write("NO\n");
    }
  }
}
