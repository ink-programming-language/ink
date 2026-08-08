// Translated from solution.cpp.

func main()
{
  var x: dynamic;
  read(x);
  var m = 0;
  var z = x;
  x += 4;
  {
    var i = 2;
    while ((i < x))
    {
      if (((x % i) != 0))
      {
        m = 1;
      } else
      {
        write(x, " ", "4");
        m = 0;
        break;
      }
      i += 1;
    }
  }
  if ((m == 1))
  {
    x = z;
    x += 6;
    {
      var i = 2;
      while ((i < x))
      {
        if (((x % i) != 0))
        {
          m = 2;
        } else
        {
          write(x, " ", "6");
          m = 3;
          break;
        }
        i += 1;
      }
    }
  }
  if ((m == 2))
  {
    x = z;
    x += 8;
    {
      var i = 2;
      while ((i < x))
      {
        if (((x % i) != 0))
        {
          m = 4;
        } else
        {
          write(x, " ", "8");
          m = 5;
          break;
        }
        i += 1;
      }
    }
  }
  if ((m == 4))
  {
    x = z;
    x += 9;
    {
      var i = 2;
      while ((i < x))
      {
        if (((x % i) != 0))
        {
          m = 6;
        } else
        {
          write(x, " ", "9");
          m = 7;
          break;
        }
        i += 1;
      }
    }
  }
}
