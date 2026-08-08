// Translated from solution.cpp.

var int_cpp = cpp_expression("#incl");

func debug(x: dynamic)
{
  cpp_macro("cerr<<#x<<\":\"<<(x)<<endl;");
}

func main()
{
  var n: dynamic;
  read(n);
  var s: dynamic;
  read(s);
  var x = 0;
  var y = 0;
  {
    var i = 0;
    while ((i < s.size()))
    {
      if (((cpp_char("a") <= s[i]) && (s[i] <= cpp_char("m"))))
      {
        x += 1;
      }
      if (((cpp_char("n") <= s[i]) && (s[i] <= cpp_char("z"))))
      {
        x -= 1;
      }
      if (((cpp_char("A") <= s[i]) && (s[i] <= cpp_char("M"))))
      {
        y += 1;
      }
      if (((cpp_char("N") <= s[i]) && (s[i] <= cpp_char("Z"))))
      {
        y -= 1;
      }
      i += 1;
    }
  }
  var ans = "";
  if ((x < 0))
  {
    ans += string_cpp(abs(x), cpp_char("n"));
  } else if ((x > 0))
  {
    ans += string_cpp(x, cpp_char("a"));
  }
  if ((y < 0))
  {
    ans += string_cpp(abs(y), cpp_char("N"));
  } else if ((y > 0))
  {
    ans += string_cpp(y, cpp_char("A"));
  }
  write(ans.size(), "\n");
  write(ans, "\n");
  return 0;
}
