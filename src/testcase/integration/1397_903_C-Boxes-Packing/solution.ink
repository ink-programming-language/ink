// Translated from solution.cpp.

func onlinejuge() -> dynamic
{
}

func isVowel(c: dynamic) -> dynamic
{
  return ((((((c == cpp_char("a")) || (c == cpp_char("e"))) || (c == cpp_char("i"))) || (c == cpp_char("o"))) || (c == cpp_char("u"))));
}

func ranklist(p1: dynamic, p2: dynamic) -> dynamic
{
  if ((p1.first > p2.first))
  {
    return 1;
  } else if ((p1.first == p2.first))
  {
    return ((p1.second < p2.second));
  } else
  {
    return 0;
  }
}

func main() -> dynamic
{
  onlinejuge();
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var map: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var max: dynamic = 0;
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    read(a);
    map[a] += 1;
    if ((max < map[a]))
    {
      max = map[a];
    }
  }
  write(max, "\n");
  return 0;
}
