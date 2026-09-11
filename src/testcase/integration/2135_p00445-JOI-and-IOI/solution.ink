// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var comp: dynamic = ["JOI", "IOI"];
  while ((cin >> s))
  {
    {
      var k: dynamic = 0;
      while ((k < 2))
      {
        var ans: dynamic = 0;
        {
          var i: dynamic = 0;
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
