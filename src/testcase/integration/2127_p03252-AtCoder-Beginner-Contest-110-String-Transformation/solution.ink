// Translated from solution.cpp.

var fr1 = cpp_array(150);

var fr2 = cpp_array(130);

func main()
{
  var s: dynamic;
  var t: dynamic;
  read(s, t);
  var y = 1;
  var mp: dynamic;
  {
    var i = 0;
    while ((i < s.size()))
    {
      fr1[s[i]] += 1;
      fr2[t[i]] += 1;
      if ((fr1[s[i]] != fr2[t[i]]))
      {
        y = 0;
        break;
      }
      i += 1;
    }
  }
  if ((y == 0))
  {
    write("No\n");
  } else
  {
    write("Yes\n");
  }
}
