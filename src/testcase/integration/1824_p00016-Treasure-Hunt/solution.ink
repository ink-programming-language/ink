// Translated from solution.cpp.

var pi: dynamic = acos(-1);

func main() -> dynamic
{
  var len: dynamic = cpp_uninitialized();
  var ang: dynamic = cpp_uninitialized();
  var ax: dynamic = 0;
  var ay: dynamic = 0;
  var vis: dynamic = 90;
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
