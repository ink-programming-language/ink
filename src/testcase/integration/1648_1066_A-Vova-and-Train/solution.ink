// Translated from solution.cpp.

func in_cpp() -> dynamic
{
  var cin: dynamic = cpp_construct("input.txt");
  ios_base.sync_with_stdio(false);
  cin.tie(0);
}

func solution() -> dynamic
{
}

func out() -> dynamic
{
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      var F: dynamic = cpp_uninitialized();
      var u: dynamic = cpp_uninitialized();
      var L: dynamic = cpp_uninitialized();
      var R: dynamic = cpp_uninitialized();
      var ans: dynamic = 0;
      read(F, u, L, R);
      ans = (((F / u) - (R / u)) + (((L - 1)) / u));
      write(ans, "\n");
      i += 1;
    }
  }
  return 0;
}
