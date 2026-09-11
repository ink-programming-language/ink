// Translated from solution.cpp.

var max1: dynamic = 100010;

var a: dynamic = cpp_array(max1);

var b: dynamic = cpp_array(max1);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = 0;
  var z: dynamic = 0;
  read(n);
  if ((n == 1))
  {
    write("NO\n");
  } else
  {
    {
      var i: dynamic = 0;
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
