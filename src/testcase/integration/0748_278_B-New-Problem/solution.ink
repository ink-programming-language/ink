// Translated from solution.cpp.

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic;
  var s: dynamic;
  var maap: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(s);
      {
        var k = 0;
        while ((k < s.size()))
        {
          {
            var h = k;
            while ((h < s.size()))
            {
              maap[s.substr(k, ((h - k) + 1))] += 1;
              h += 1;
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  s.clear();
  s = "a";
  while (maap[s])
  {
    s[(s.size() - 1)] += 1;
    if (((s[(s.size() - 1)] > cpp_char("z")) && (s.size() == 1)))
    {
      s = "aa";
    } else if (((s.size() == 2) && (s[(s.size() - 1)] > cpp_char("z"))))
    {
      s[0] += 1;
      s[(s.size() - 1)] = cpp_char("a");
      if ((s[0] > cpp_char("z")))
      {
        s = "aaa";
      }
    }
  }
  write(s, "\n");
  return 0;
}
