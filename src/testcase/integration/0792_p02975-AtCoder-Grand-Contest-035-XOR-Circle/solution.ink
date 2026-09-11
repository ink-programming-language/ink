// Translated from solution.cpp.

var nl: dynamic = cpp_char("\n");

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var x: dynamic = 0;
  for (var i: dynamic in a)
  {
    read(i);
    x ^= i;
  }
  puts( ((x == 0)) ? "Yes" : "No");
  return 0;
}
