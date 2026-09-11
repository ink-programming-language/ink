// Translated from solution.cpp.

func main() -> dynamic
{
  var str: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  getline(cin, str);
  var len: dynamic = str.length();
  {
    var i: dynamic = 0;
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
