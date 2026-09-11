// Translated from solution.cpp.

func f(a: dynamic, v: dynamic, cur_max: dynamic) -> dynamic
{
  return max(cur_max, max(abs((a[0] - v)), abs((a.back() - v))));
}

func main() -> dynamic
{
  srand(chrono.duration_cast(chrono.high_resolution_clock.now().time_since_epoch()).count());
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    for (var e: dynamic in a)
    {
      read(e);
    }
    var cur_max: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        if (((a[(i - 1)] == -1) || (a[i] == -1)))
        {
          i += 1;
          continue;
        }
        cur_max = max(cur_max, abs((a[i] - a[(i - 1)])));
        i += 1;
      }
    }
    var z: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((a[i] != -1))
        {
          i += 1;
          continue;
        }
        if (((i > 0) && (a[(i - 1)] != -1)))
        {
          z.push_back(a[(i - 1)]);
        }
        if ((((i + 1) < n) && (a[(i + 1)] != -1)))
        {
          z.push_back(a[(i + 1)]);
        }
        i += 1;
      }
    }
    if (z.empty())
    {
      write("0 0\n");
      continue;
    }
    sort(z.begin(), z.end());
    var q: dynamic = (((z[0] + cpp_cast(z.back()))) / 2);
    write(f(z, q, cur_max), cpp_char(" "), q, cpp_char("\n"));
  }
  return 0;
}
