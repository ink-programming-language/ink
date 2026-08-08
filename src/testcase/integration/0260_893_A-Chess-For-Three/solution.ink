// Translated from solution.cpp.

var a = cpp_array(1005);

func main()
{
  var n: dynamic;
  while ((cin >> n))
  {
    var x = 1;
    var y = 1;
    var z = 0;
    var k = 0;
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        if ((((a[i] == 1) && (x == 1)) && (y == 1)))
        {
          y = 0;
          z = 1;
        } else if ((((a[i] == 2) && (x == 1)) && (y == 1)))
        {
          x = 0;
          z = 1;
        } else if ((((a[i] == 1) && (x == 1)) && (z == 1)))
        {
          z = 0;
          y = 1;
        } else if ((((a[i] == 3) && (x == 1)) && (z == 1)))
        {
          x = 0;
          y = 1;
        } else if ((((a[i] == 2) && (y == 1)) && (z == 1)))
        {
          z = 0;
          x = 1;
        } else if ((((a[i] == 3) && (y == 1)) && (z == 1)))
        {
          y = 0;
          x = 1;
        } else
        {
          k = 1;
          break;
        }
        i += 1;
      }
    }
    if ((k == 1))
    {
      write("NO", "\n");
    } else if ((k == 0))
    {
      write("YES", "\n");
    }
  }
  return 0;
}
