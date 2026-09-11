// Translated from solution.cpp.

func abs(a: dynamic) -> dynamic
{
  return  ((a < 0)) ? (-a) : a;
}

func sqr(a: dynamic) -> dynamic
{
  return (a * a);
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var first: dynamic = (((sqrt(((8.0 * n) + 1)) - 1)) / 2);
  var out: dynamic = (((first - floor(first))) * ((first + 1)));
  if ((out == 0))
  {
    write(cpp_cast(first));
  } else
  {
    write(out);
  }
}
