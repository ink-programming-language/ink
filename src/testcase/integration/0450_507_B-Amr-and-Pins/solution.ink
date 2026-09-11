// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var r: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var x1: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  read(r, x, y, x1, y1);
  var ans: dynamic = cpp_uninitialized();
  ans = ceil((sqrt((pow((x - x1), 2) + pow((y - y1), 2))) / ((2 * r))));
  write(ans);
  return 0;
}
