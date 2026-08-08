// Translated from solution.cpp.

func abs(a: dynamic)
{
  return if ((a < 0)) (-a) else a;
}

func sqr(a: dynamic)
{
  return (a * a);
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  read(n);
  var first = (((sqrt(((8.0 * n) + 1)) - 1)) / 2);
  var out = (((first - floor(first))) * ((first + 1)));
  if ((out == 0))
  {
    write(cpp_cast(first));
  } else
  {
    write(out);
  }
}
