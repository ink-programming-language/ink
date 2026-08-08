// Translated from solution.cpp.

func main()
{
  var sent = cpp_array(1200);
  var x: dynamic;
  gets(sent);
  {
    x = 0;
    while ((sent[x] != cpp_char("\u{0}")))
    {
      if (isupper(sent[x]))
      {
        sent[x] = tolower(sent[x]);
      } else
      {
        sent[x] = toupper(sent[x]);
      }
      x += 1;
    }
  }
  puts(sent);
  return 0;
}
