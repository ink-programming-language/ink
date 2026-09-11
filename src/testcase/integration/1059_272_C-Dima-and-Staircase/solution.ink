// Translated from solution.cpp.

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var PI: dynamic = acos(-1);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n);
  var arr: dynamic = cpp_array(n);
  for (var el: dynamic in arr)
  {
    read(el);
  }
  read(m);
  var hi: dynamic = 0;
  while (cpp_update(m, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    read(x, y);
    x -= 1;
    var res: dynamic = max(hi, arr[x]);
    write(res, "\n");
    hi = max((res + y), hi);
  }
  return 0;
}
