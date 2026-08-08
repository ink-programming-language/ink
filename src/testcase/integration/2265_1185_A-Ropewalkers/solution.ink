// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  read(a, b, c, d);
  if ((a > b))
  {
    swap(a, b);
  }
  if ((b > c))
  {
    swap(b, c);
  }
  if ((a > b))
  {
    swap(a, b);
  }
  write(max(0, (((2 * d) - min(d, ((b - a)))) - min(d, ((c - b))))), cpp_char("\n"));
  return 0;
}
