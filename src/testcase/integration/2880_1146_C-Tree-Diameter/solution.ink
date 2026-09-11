// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  a.clear();
  b.clear();
  var ans: dynamic = 0;
  {
    var i: dynamic = 2;
    while (((i / 2) < n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          {
            var k: dynamic = j;
            while ((k < (j + (i / 2))))
            {
              if ((k > n))
              {
                break;
              }
              a.push_back(k);
              k += 1;
            }
          }
          {
            var k: dynamic = (j + (i / 2));
            while ((k < (j + i)))
            {
              if ((k > n))
              {
                break;
              }
              b.push_back(k);
              k += 1;
            }
          }
          j += i;
        }
      }
      write(a.size(), " ", b.size(), " ");
      {
        var k: dynamic = 0;
        while ((k < a.size()))
        {
          write(a[k], " ");
          k += 1;
        }
      }
      {
        var k: dynamic = 0;
        while ((k < b.size()))
        {
          write(b[k], " ");
          k += 1;
        }
      }
      write("\n");
      a.clear();
      b.clear();
      cout.flush();
      var in_cpp: dynamic = cpp_uninitialized();
      read(in_cpp);
      ans = max(ans, in_cpp);
      i *= 2;
    }
  }
  write(-1, " ", ans, "\n");
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
