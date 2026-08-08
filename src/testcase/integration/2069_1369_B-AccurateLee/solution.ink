// Translated from solution.cpp.

var t: dynamic;

var n: dynamic;

var last: dynamic;

var s: dynamic;

var wynik: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, s);
    wynik = "";
    var ile0 = 0;
    var ile1 = 0;
    {
      var i = 0;
      while (((i < s.size()) && (s[i] != cpp_char("1"))))
      {
        ile0 += 1;
        i += 1;
      }
    }
    {
      var i = (s.size() - 1);
      while (((i >= 0) && (s[i] != cpp_char("0"))))
      {
        ile1 += 1;
        i -= 1;
      }
    }
    {
      var i = 0;
      while ((i < ile0))
      {
        write(cpp_char("0"));
        i += 1;
      }
    }
    if ((n > (ile0 + ile1)))
    {
      write(cpp_char("0"));
    }
    {
      var i = 0;
      while ((i < ile1))
      {
        write(cpp_char("1"));
        i += 1;
      }
    }
    write(wynik, "\n");
  }
}
