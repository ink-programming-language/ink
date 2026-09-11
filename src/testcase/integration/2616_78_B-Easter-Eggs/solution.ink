// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  var s: dynamic = "ROYGBIV";
  var i: dynamic = 0;
  while (cpp_update(t, "--"))
  {
    write(s[cpp_update(i, "++")]);
    if ((i == 7))
    {
      i = 3;
    }
  }
  return 0;
}
