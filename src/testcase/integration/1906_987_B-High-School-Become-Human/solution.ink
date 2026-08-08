// Translated from solution.cpp.

func main()
{
  var x: dynamic;
  var y: dynamic;
  read(x, y);
  if ((x == y))
  {
    write("=");
  } else if ((x < y))
  {
    if (((x >= 3) && (y > 3)))
    {
      write(">");
    } else if (((x == 2) && (y == 3)))
    {
      write("<");
    } else if (((((x == 2) && (y == 4))) || (((x == 2) && (y == 8)))))
    {
      write("=");
    } else if (((x == 2) && (y > 3)))
    {
      write(">");
    } else
    {
      write("<");
    }
  } else
  {
    var c = y;
    y = x;
    x = c;
    if (((x >= 3) && (y > 3)))
    {
      write("<");
    } else if (((x == 2) && (y == 3)))
    {
      write(">");
    } else if (((((x == 2) && (y == 4))) || (((x == 2) && (y == 8)))))
    {
      write("=");
    } else if (((x == 2) && (y > 3)))
    {
      write("<");
    } else
    {
      write(">");
    }
  }
  return 0;
}
