// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var i: dynamic;
    read(n);
    var l = cpp_array(n);
    var r = cpp_array(n);
    {
      i = 0;
      while ((i < n))
      {
        read(l[i], r[i]);
        i += 1;
      }
    }
    if ((n == 1))
    {
      write(0, "\n");
    } else
    {
      sort(l, (l + n));
      sort(r, (r + n));
      if ((l[(n - 1)] > r[0]))
      {
        var ans = abs((r[0] - l[(n - 1)]));
        write(ans, "\n");
      } else
      {
        write(0, "\n");
      }
    }
  }
}
