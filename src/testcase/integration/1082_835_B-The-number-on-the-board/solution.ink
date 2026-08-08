// Translated from solution.cpp.

func out(x: dynamic)
{
  write(x, "\n");
  exit(0);
}

var maxn = (1e6 + 5);

var k: dynamic;

var s: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(k);
  read(s);
  var tot = 0;
  sort(s.begin(), s.end());
  for (var c in s)
  {
    tot += (c - cpp_char("0"));
  }
  var x = 0;
  for (var c in s)
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
