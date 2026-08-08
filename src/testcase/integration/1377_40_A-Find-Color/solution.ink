// Translated from solution.cpp.

var x: dynamic;

var y: dynamic;

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(x, y);
  var dist = sqrt((((x * x) + (y * y))));
  if (((((y >= 0) && (x >= 0))) || (((y < 0) && (x < 0)))))
  {
    if (((int_cpp(dist) % 2) && (int_cpp(dist) != dist)))
    {
      return cpp_comma((cout << "white"), 0);
    } else
    {
      return cpp_comma((cout << "black"), 0);
    }
  } else
  {
    if (((int_cpp(dist) % 2) || (int_cpp(dist) == dist)))
    {
      return cpp_comma((cout << "black"), 0);
    } else
    {
      return cpp_comma((cout << "white"), 0);
    }
  }
  return 0;
}
