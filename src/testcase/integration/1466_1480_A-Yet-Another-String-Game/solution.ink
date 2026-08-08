// Translated from solution.cpp.

var inf = (1e9 + 7);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var s: dynamic;
    read(s);
    var a = true;
    {
      var i = 0;
      while ((i < s.length()))
      {
        if (a)
        {
          if ((s[i] != cpp_char("a")))
          {
            s[i] = cpp_char("a");
          } else
          {
            s[i] = cpp_char("b");
          }
          a = false;
        } else
        {
          if ((s[i] != cpp_char("z")))
          {
            s[i] = cpp_char("z");
          } else
          {
            s[i] = cpp_char("y");
          }
          a = true;
        }
        i += 1;
      }
    }
    write(s, "\n");
  }
}
