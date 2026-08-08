// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  read(s);
  var ans = 105;
  {
    var i = 0;
    while ((i < s.size()))
    {
      var cnt = 0;
      var maxcnt = 0;
      {
        var j = (s.size() - 1);
        while ((j >= 0))
        {
          if ((s[i] == s[j]))
          {
            cnt = 0;
          } else
          {
            cnt += 1;
          }
          maxcnt = max(maxcnt, cnt);
          j -= 1;
        }
      }
      ans = min(ans, maxcnt);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
