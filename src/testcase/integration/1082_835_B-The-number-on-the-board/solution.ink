// Translated from solution.cpp.

func out(x: dynamic) -> dynamic
{
  write(x, "\n");
  exit(0);
}

var maxn: dynamic = (1e6 + 5);

var k: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(k);
  read(s);
  var tot: dynamic = 0;
  sort(s.begin(), s.end());
  for (var c: dynamic in s)
  {
    tot += (c - cpp_char("0"));
  }
  var x: dynamic = 0;
  for (var c: dynamic in s)
  {
    if ((tot >= k))
    {
      out(x);
    }
    x += 1;
    tot -= (c - cpp_char("0"));
    tot += 9;
  }
  assert((tot >= k));
  out(x);
  return 0;
}
