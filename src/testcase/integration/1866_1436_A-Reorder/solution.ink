// Translated from solution.cpp.

func fast() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  return 0;
}

func main() -> dynamic
{
  fast();
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    var arr: dynamic = cpp_array(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
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
