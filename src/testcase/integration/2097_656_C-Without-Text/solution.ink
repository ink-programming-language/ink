// Translated from solution.cpp.

func main()
{
  var str: dynamic;
  var ans = 0;
  getline(cin, str);
  var len = str.length();
  {
    var i = 0;
    while ((i <= (len - 1)))
    {
      if (isupper(str[i]))
      {
        ans += (str[i] - 64);
      }
      if (islower(str[i]))
      {
        ans -= (str[i] - 96);
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
