// Translated from solution.cpp.

func size(a: dynamic) -> dynamic
{
  return cpp_cast(a.size());
}

func sqr(a: dynamic) -> dynamic
{
  return (a * a);
}

func isLowercase(ch: dynamic) -> dynamic
{
  return ((ch >= cpp_char("a")) && (ch <= cpp_char("z")));
}

func isUppercase(ch: dynamic) -> dynamic
{
  return (!isLowercase(ch));
}

func toLowercase(ch: dynamic) -> dynamic
{
  if (isUppercase(ch))
  {
    ch = ((ch - cpp_char("A")) + cpp_char("a"));
  }
  return ch;
}

func toUppercase(ch: dynamic) -> dynamic
{
  if (isLowercase(ch))
  {
    ch = ((ch - cpp_char("a")) + cpp_char("A"));
  }
  return ch;
}

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(s, n);
  {
    var i: dynamic = (0);
    while ((i < (size(s))))
    {
      s[i] = toLowercase(s[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < (size(s))))
    {
      if ((int_cpp(s[i]) < (n + 97)))
      {
        s[i] = toUppercase(s[i]);
      }
      i += 1;
    }
  }
  write(s, "\n");
}
