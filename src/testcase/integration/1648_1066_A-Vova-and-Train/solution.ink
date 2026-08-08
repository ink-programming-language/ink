// Translated from solution.cpp.

func in_cpp()
{
  var cin = cpp_construct("input.txt");
  ios_base.sync_with_stdio(false);
  cin.tie(0);
}

func solution()
{
}

func out()
{
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic;
  read(t);
  {
    var i = 0;
    while ((i < t))
    {
      var F: dynamic;
      var u: dynamic;
      var L: dynamic;
      var R: dynamic;
      var ans = 0;
      read(F, u, L, R);
      ans = (((F / u) - (R / u)) + (((L - 1)) / u));
      write(ans, "\n");
      i += 1;
    }
  }
  return 0;
}
