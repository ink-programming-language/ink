// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var k: dynamic;

var l: dynamic;

var r: dynamic;

var t: dynamic;

var qq: dynamic;

var a = cpp_array(1000009);

func main()
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
      var i = 0;
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
      var i = 0;
      while ((i < n))
      {
        {
          var j = (i + 1);
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
