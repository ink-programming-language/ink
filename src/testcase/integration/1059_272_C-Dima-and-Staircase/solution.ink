// Translated from solution.cpp.

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var PI = acos(-1);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n);
  var arr = cpp_array(n);
  for (var el in arr)
  {
    read(el);
  }
  read(m);
  var hi = 0;
  while (cpp_update(m, "--"))
  {
    var x: dynamic;
    var y: dynamic;
    read(x, y);
    x -= 1;
    var res = max(hi, arr[x]);
    write(res, "\n");
    hi = max((res + y), hi);
  }
  return 0;
}
