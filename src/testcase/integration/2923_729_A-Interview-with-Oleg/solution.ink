// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(51);

func Filler() -> dynamic
{
  a[1] = "ogo";
  {
    var i: dynamic = 2;
    while ((i <= 50))
    {
      a[i] = (a[(i - 1)] + "go");
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  Filler();
  read(n);
  read(s);
  while ((s.size() != 0))
  {
    if (((((s[0] == cpp_char("o"))) && ((s[1] == cpp_char("g")))) && ((s[2] == cpp_char("o")))))
    {
      var i: dynamic = 1;
      while ((s.find(a[i], 0) == 0))
      {
        i += 1;
      }
      s.erase(0, a[(i - 1)].size());
      write("***");
    } else
    {
      write(s[0]);
      s.erase(0, 1);
    }
  }
  return 0;
}
