// Translated from solution.cpp.

func loop(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func rep(i: dynamic, a: dynamic) -> dynamic
{
  return cpp_expression("#include<io");
}

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  var tmp: dynamic = false;
  {
    var i: dynamic = (s.size() - t.size());
    while ((i >= 0))
    {
      var check: dynamic = true;
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
  write(( ((tmp)) ? s : "UNRESTORABLE"), "\n");
  return 0;
}
