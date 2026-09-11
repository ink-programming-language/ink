// Translated from solution.cpp.

func main() -> dynamic
{
  var home: dynamic = cpp_array(30);
  var away: dynamic = cpp_array(30);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(home[i]);
      read(away[i]);
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          if ((home[i] == away[j]))
          {
            cnt += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(cnt, cpp_char("\n"));
  return 0;
}
