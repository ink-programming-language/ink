// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var i: dynamic = cpp_uninitialized();
    read(n);
    var l: dynamic = cpp_array(n);
    var r: dynamic = cpp_array(n);
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
        var ans: dynamic = abs((r[0] - l[(n - 1)]));
        write(ans, "\n");
      } else
      {
        write(0, "\n");
      }
    }
  }
}
