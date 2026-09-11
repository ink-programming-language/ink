// Translated from solution.cpp.

var int_cpp: dynamic = cpp_expression("#incl");

func debug(x: dynamic) -> dynamic
{
  cpp_macro("cerr<<#x<<\":\"<<(x)<<endl;");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var x: dynamic = 0;
  var y: dynamic = 0;
  {
    var i: dynamic = 0;
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
  var ans: dynamic = "";
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
