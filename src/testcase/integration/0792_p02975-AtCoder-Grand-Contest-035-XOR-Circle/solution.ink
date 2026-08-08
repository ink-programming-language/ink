// Translated from solution.cpp.

var nl = cpp_char("\n");

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  read(n);
  var x = 0;
  for (var i in a)
  {
    read(i);
    x ^= i;
  }
  puts(if ((x == 0)) "Yes" else "No");
  return 0;
}
