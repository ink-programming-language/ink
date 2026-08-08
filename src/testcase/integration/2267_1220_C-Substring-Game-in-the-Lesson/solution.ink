// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var str: dynamic;
  read(str);
  var n = cpp_cast(str.size());
  var min_ind = 0;
  write("Mike", "\n");
  var mini = str[0];
  {
    var i = 1;
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
