// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var str: dynamic = cpp_uninitialized();
  read(str);
  var n: dynamic = cpp_cast(str.size());
  var min_ind: dynamic = 0;
  write("Mike", "\n");
  var mini: dynamic = str[0];
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      if ((mini < str[i]))
      {
        write("Ann", "\n");
      } else
      {
        mini = str[i];
        write("Mike", "\n");
      }
      i += 1;
    }
  }
  return 0;
}
