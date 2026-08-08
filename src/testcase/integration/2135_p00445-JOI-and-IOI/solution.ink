// Translated from solution.cpp.

func main()
{
  var s: dynamic;
  var comp = ["JOI", "IOI"];
  while ((cin >> s))
  {
    {
      var k = 0;
      while ((k < 2))
      {
        var ans = 0;
        {
          var i = 0;
          while (((cpp_assign(i, "=", s.find(comp[k], i))) != string_cpp.npos))
          {
            ans += 1;
            i += 1;
          }
        }
        write(ans, "\n");
        k += 1;
      }
    }
  }
  return 0;
}
