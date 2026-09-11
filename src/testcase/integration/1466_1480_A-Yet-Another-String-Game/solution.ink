// Translated from solution.cpp.

var inf: dynamic = (1e9 + 7);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var s: dynamic = cpp_uninitialized();
    read(s);
    var a: dynamic = true;
    {
      var i: dynamic = 0;
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
