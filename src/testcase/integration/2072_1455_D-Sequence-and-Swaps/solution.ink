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
    var x: dynamic;
    read(n, x);
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    if (is_sorted(a.begin(), a.end()))
    {
      write(0, cpp_char("\n"));
      continue;
    }
    var ok = false;
    var cnt = 0;
    {
      var i = 0;
      while ((i < n))
      {
        if ((a[i] > x))
        {
          swap(a[i], x);
          cnt += 1;
        }
        if (is_sorted(a.begin(), a.end()))
        {
          write(cnt, cpp_char("\n"));
          ok = true;
          break;
        }
        i += 1;
      }
    }
    if (ok)
    {
      continue;
    }
    write(-1, cpp_char("\n"));
  }
  return 0;
}
