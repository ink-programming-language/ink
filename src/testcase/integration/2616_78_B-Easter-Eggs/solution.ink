// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  read(t);
  var s = "ROYGBIV";
  var i = 0;
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
