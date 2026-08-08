// Translated from solution.cpp.

func loop(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func rep(i: dynamic, a: dynamic)
{
  return cpp_expression("#include<io");
}

func main()
{
  var s: dynamic;
  var t: dynamic;
  read(s, t);
  var tmp = false;
  {
    var i = (s.size() - t.size());
    while ((i >= 0))
    {
      var check = true;
      rep(j, t.size());
      {
        if (cpp_binary((s[(i + j)] == cpp_char("?")), "or", (s[(i + j)] == t[j])))
        {
          i -= 1;
          continue;
        }
        check = false;
        break;
      }
      if ((check && (!tmp)))
      {
        rep(j, t.size())[(i + j)] = t[j];
        tmp = true;
      }
      i -= 1;
    }
  }
  rep(i, s.size());
  if ((s[i] == cpp_char("?")))
  {
    s[i] = cpp_char("a");
  }
  write((if ((tmp)) s else "UNRESTORABLE"), "\n");
  return 0;
}
