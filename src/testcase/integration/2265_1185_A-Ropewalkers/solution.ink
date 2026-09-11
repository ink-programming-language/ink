// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
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
