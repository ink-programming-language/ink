// Translated from solution.cpp.

func fr(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);++i)");
}

func Fr(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=(n);++i)");
}

func ifr(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(n)-1;i>=0;--i)");
}

func iFr(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(n);i>0;--i)");
}

func main() -> dynamic
{
  cin.tie(null);
  ios.sync_with_stdio(false);
  var in_cpp: dynamic = cpp_uninitialized();
  var out: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = [];
  var m: dynamic = LLONG_MAX;
  (in_cpp >> n);
  for (var i: dynamic in a)
  {
    (in_cpp >> i);
    s += i;
    m = min(m, i);
  }
  if ((n & 1))
  {
    return cpp_comma(puts( ((s % 2)) ? "First" : "Second"), 0);
  }
  if ((m & 1))
  {
    return cpp_comma(puts("First"), 0);
  }
  puts( ((s % 2)) ? "First" : "Second");
}
