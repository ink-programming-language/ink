// Translated from solution.cpp.

func main()
{
  var home = cpp_array(30);
  var away = cpp_array(30);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(home[i]);
      read(away[i]);
      i += 1;
    }
  }
  var cnt = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
