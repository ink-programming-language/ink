// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var qq: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000009);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(qq);
  while (cpp_update(qq, "--"))
  {
    read(n, k);
    m = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        if ((a[i] == k))
        {
          m = 1;
        }
        if ((a[i] >= k))
        {
          a[i] = 1;
        } else if ((a[i] < k))
        {
          a[i] = 0;
        }
        i += 1;
      }
    }
    if ((m != 1))
    {
      write("no\n");
      continue;
    }
    m = 0;
    if ((n == 1))
    {
      write("yes\n");
      continue;
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            if (((j - i) > 2))
            {
              break;
            }
            if (((a[i] != 0) && (a[j] != 0)))
            {
              m = 1;
              break;
            }
            j += 1;
          }
        }
        if ((m == 1))
        {
          break;
        }
        i += 1;
      }
    }
    if ((m == 1))
    {
      write("yes\n");
    } else
    {
      write("no\n");
    }
  }
  return 0;
}
