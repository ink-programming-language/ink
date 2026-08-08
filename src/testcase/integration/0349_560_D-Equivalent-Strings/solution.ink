// Translated from solution.cpp.

func minEquivalent(s: dynamic)
{
  if ((s.length() % 2))
  {
    return s;
  }
  var s1 = minEquivalent(s.substr(0, (s.length() / 2)));
  var s2 = minEquivalent(s.substr((s.length() / 2), (s.length() / 2)));
  if ((s1 < s2))
  {
    return (s1 + s2);
  } else
  {
    return (s2 + s1);
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  if ((minEquivalent(a) == minEquivalent(b)))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
