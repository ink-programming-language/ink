// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  read(s);
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((s[i] == cpp_char("+")))
      {
        i += 1;
        continue;
      }
      {
        var j: dynamic = 0;
        while ((j < s.size()))
        {
          if ((s[j] == cpp_char("+")))
          {
            j += 1;
            continue;
          }
          if ((s[i] < s[j]))
          {
            swap(s[j], s[i]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      write(s[i]);
      i += 1;
    }
  }
  return 0;
}
