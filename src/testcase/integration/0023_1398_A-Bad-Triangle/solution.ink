// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var tt: dynamic;
  read(tt);
  while (cpp_update(tt, "--"))
  {
    var n: dynamic;
    read(n);
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    if (((a[0] + a[1]) <= a.back()))
    {
      write("1 2 ", n, cpp_char("\n"));
      cpp_goto("goto nx;");
    }
    write("-1\n");
  }
  return 0;
}
