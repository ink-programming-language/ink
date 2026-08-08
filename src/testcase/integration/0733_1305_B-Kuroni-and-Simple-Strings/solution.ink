// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  read(s);
  var a: dynamic;
  var b: dynamic;
  var i = 0;
  var j = (s.size() - 1);
  while ((i < j))
  {
    while (((i < s.size()) && (s[i] == cpp_char(")"))))
    {
      i += 1;
    }
    while (((j >= 0) && (s[j] == cpp_char("("))))
    {
      j -= 1;
    }
    if ((((i < s.size()) && (j >= 0)) && (i < j)))
    {
      a.push_back((i + 1));
      b.push_back((j + 1));
      i += 1;
      j -= 1;
    }
  }
  if (a.empty())
  {
    write(0, "\n");
    return 0;
  }
  write(1, "\n");
  write((2 * a.size()), "\n");
  for (var i in a)
  {
    write(i, " ");
  }
  reverse(b.begin(), b.end());
  for (var i in b)
  {
    write(i, " ");
  }
  write("\n");
  return 0;
}
