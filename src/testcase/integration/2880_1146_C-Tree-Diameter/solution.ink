// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  read(n);
  var a: dynamic;
  var b: dynamic;
  a.clear();
  b.clear();
  var ans = 0;
  {
    var i = 2;
    while (((i / 2) < n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          {
            var k = j;
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
            var k = (j + (i / 2));
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
        var k = 0;
        while ((k < a.size()))
        {
          write(a[k], " ");
          k += 1;
        }
      }
      {
        var k = 0;
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
      var in_cpp: dynamic;
      read(in_cpp);
      ans = max(ans, in_cpp);
      i *= 2;
    }
  }
  write(-1, " ", ans, "\n");
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
