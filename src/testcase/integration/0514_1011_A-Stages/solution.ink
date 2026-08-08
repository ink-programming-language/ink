// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var s: dynamic;
  read(s);
  sort(s.begin(), s.end());
  var ans: dynamic;
  ans += s[0];
  k -= 1;
  {
    var i = 1;
    while (((i < s.length()) && (k > 0)))
    {
      if (((k > 0) && ((s[i] - ans[(ans.length() - 1)]) > 1)))
      {
        ans += s[i];
        k -= 1;
      }
      i += 1;
    }
  }
  if ((k == 0))
  {
    var count = 0;
    {
      var i = 0;
      while ((i < ans.length()))
      {
        count += (ans[i] - 96);
        i += 1;
      }
    }
    write(count, cpp_char("\n"));
  } else
  {
    write("-1\n");
  }
}
