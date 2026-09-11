// Translated from solution.cpp.

var fr1: dynamic = cpp_array(150);

var fr2: dynamic = cpp_array(130);

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  var y: dynamic = 1;
  var mp: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
