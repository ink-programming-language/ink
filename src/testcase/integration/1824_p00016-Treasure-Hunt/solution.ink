// Translated from solution.cpp.

var pi = acos(-1);

func main()
{
  var len: dynamic;
  var ang: dynamic;
  var ax = 0;
  var ay = 0;
  var vis = 90;
  while (scanf("%d,%d", (&len), (&ang)))
  {
    if (((len + ang) == 0))
    {
      break;
    }
    ax += (len * cos(((pi * vis) / 180)));
    ay += (len * sin(((pi * vis) / 180)));
    vis -= ang;
  }
  write(cpp_cast(ax), "\n");
  write(cpp_cast(ay), "\n");
}
