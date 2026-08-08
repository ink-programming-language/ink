// Translated from solution.cpp.

func main()
{
  var s = cpp_array(4);
  {
    var i = 0;
    while ((i < 4))
    {
      read(s[i]);
      i += 1;
    }
  }
  sort(s, (s + 4));
  if ((((s[0] == s[1]) && (s[2] == s[3])) && (s[1] != s[2])))
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
}
