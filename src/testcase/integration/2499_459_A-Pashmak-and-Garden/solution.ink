// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  read(x1, y1, x2, y2);
  if ((((x1 != x2) && (y1 != y2)) && (abs((x1 - x2)) != abs((y1 - y2)))))
  {
    write(-1, "\n");
  } else if ((x1 == x2))
  {
    write((x1 + abs((y1 - y2))), " ", y1, " ", (x2 + abs((y1 - y2))), " ", y2, "\n");
  } else if ((y1 == y2))
  {
    write(x1, " ", (y1 + abs((x1 - x2))), " ", x2, " ", (y2 + abs((x1 - x2))), "\n");
  } else
  {
    write(x1, " ", y2, " ", x2, " ", y1, "\n");
  }
  return 0;
}
