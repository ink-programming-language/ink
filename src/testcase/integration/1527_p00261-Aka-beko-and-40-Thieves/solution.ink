// Translated from solution.cpp.

var s: dynamic;

func dfs(c: dynamic, p: dynamic)
{
  if ((p == s.size()))
  {
    return (c == cpp_char("B"));
  }
  if ((c == cpp_char("A")))
  {
    if ((s[p] == cpp_char("0")))
    {
      if (dfs(cpp_char("X"), (p + 1)))
      {
        return 1;
      }
    } else if (dfs(cpp_char("Y"), (p + 1)))
    {
      return 1;
    }
  }
  if ((c == cpp_char("B")))
  {
    if ((s[p] == cpp_char("0")))
    {
      if (dfs(cpp_char("Y"), (p + 1)))
      {
        return 1;
      }
    } else if (dfs(cpp_char("X"), (p + 1)))
    {
      return 1;
    }
  }
  if ((c == cpp_char("X")))
  {
    if ((s[p] == cpp_char("1")))
    {
      if (dfs(cpp_char("Z"), (p + 1)))
      {
        return 1;
      }
    }
  }
  if ((c == cpp_char("Y")))
  {
    if ((s[p] == cpp_char("0")))
    {
      if (dfs(cpp_char("X"), (p + 1)))
      {
        return 1;
      }
    }
  }
  if ((c == cpp_char("W")))
  {
    if ((s[p] == cpp_char("0")))
    {
      if (dfs(cpp_char("B"), (p + 1)))
      {
        return 1;
      }
    } else if (dfs(cpp_char("Y"), (p + 1)))
    {
      return 1;
    }
  }
  if ((c == cpp_char("Z")))
  {
    if ((s[p] == cpp_char("0")))
    {
      if (dfs(cpp_char("W"), (p + 1)))
      {
        return 1;
      }
    } else if (dfs(cpp_char("B"), (p + 1)))
    {
      return 1;
    }
  }
  return 0;
}

func main()
{
  while (((cin >> s) && (s != "#")))
  {
    write((if (dfs(cpp_char("A"), 0)) "Yes" else "No"), "\n");
  }
}
