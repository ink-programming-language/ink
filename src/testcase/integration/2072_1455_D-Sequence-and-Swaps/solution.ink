// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var tt: dynamic = cpp_uninitialized();
  read(tt);
  while (cpp_update(tt, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    read(n, x);
    {
      var i: dynamic = 0;
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
    var ok: dynamic = false;
    var cnt: dynamic = 0;
    {
      var i: dynamic = 0;
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
