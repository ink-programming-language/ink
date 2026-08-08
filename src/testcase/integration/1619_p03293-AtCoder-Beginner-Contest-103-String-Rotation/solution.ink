// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  var t: dynamic;
  read(s, t);
  var sz = s.size();
  {
    var i = 0;
    while ((i < sz))
    {
      if ((s.substr(i) == t))
      {
        write("Yes", "\n");
        return 0;
      }
      s += s[i];
      i += 1;
    }
  }
  write("No", "\n");
}
