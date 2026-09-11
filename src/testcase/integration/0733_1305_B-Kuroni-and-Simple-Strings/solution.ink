// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var i: dynamic = 0;
  var j: dynamic = (s.size() - 1);
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
  for (var i: dynamic in a)
  {
    write(i, " ");
  }
  reverse(b.begin(), b.end());
  for (var i: dynamic in b)
  {
    write(i, " ");
  }
  write("\n");
  return 0;
}
