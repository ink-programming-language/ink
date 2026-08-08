// Translated from solution.cpp.

func FOR(i: dynamic, st: dynamic, n: dynamic)
{
  cpp_macro("for (int i = st; i < n; i++)");
}

var INF = (1e9 + 100);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var s: dynamic;
    read(s);
    var n = s.size();
    var ans = 0;
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if ((((i < (n - 2)) && (s[i] == s[(i + 1)])) && (s[i] == s[(i + 2)])))
        {
          ans += 2;
          i += 2;
        } else if ((s[i] == s[(i + 1)]))
        {
          ans += 1;
          i += 1;
        } else if (((i < (n - 2)) && (s[i] == s[(i + 2)])))
        {
          ans += 1;
          if (((i < (n - 3)) && (s[(i + 1)] == s[(i + 3)])))
          {
            ans += 1;
            i += 3;
          } else
          {
            i += 2;
          }
        }
        i += 1;
      }
    }
    write(ans, cpp_char("\n"));
  }
  return 0;
}
