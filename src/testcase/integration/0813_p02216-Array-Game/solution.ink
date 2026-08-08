// Translated from solution.cpp.

func fr(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);++i)");
}

func Fr(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=1;i<=(n);++i)");
}

func ifr(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(n)-1;i>=0;--i)");
}

func iFr(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(n);i>0;--i)");
}

func main()
{
  cin.tie(null);
  ios.sync_with_stdio(false);
  var in_cpp: dynamic;
  var out: dynamic;
  var n: dynamic;
  var s = [];
  var m = LLONG_MAX;
  (in_cpp >> n);
  for (var i in a)
  {
    (in_cpp >> i);
    s += i;
    m = min(m, i);
  }
  if ((n & 1))
  {
    return cpp_comma(puts(if ((s % 2)) "First" else "Second"), 0);
  }
  if ((m & 1))
  {
    return cpp_comma(puts("First"), 0);
  }
  puts(if ((s % 2)) "First" else "Second");
}
