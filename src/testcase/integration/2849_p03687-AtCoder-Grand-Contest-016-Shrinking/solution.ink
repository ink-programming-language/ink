// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var ans: dynamic = 105;
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      var cnt: dynamic = 0;
      var maxcnt: dynamic = 0;
      {
        var j: dynamic = (s.size() - 1);
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
