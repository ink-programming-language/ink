// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  {
    var i: dynamic = 1;
    var tmp: dynamic = cpp_uninitialized();
    var stop: dynamic = 0;
    while ((i <= n))
    {
      read(tmp);
      stop = max(stop, tmp);
      if ((stop == i))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  printf("%d", ans);
  return 0;
}
