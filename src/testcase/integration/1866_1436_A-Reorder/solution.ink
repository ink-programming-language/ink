// Translated from solution.cpp.

func fast()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  return 0;
}

func main()
{
  fast();
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    var arr = cpp_array(n);
    {
      var i = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        m -= arr[i];
        i += 1;
      }
    }
    if ((m == 0))
    {
      write("YES\n");
    } else
    {
      write("NO\n");
    }
  }
}
