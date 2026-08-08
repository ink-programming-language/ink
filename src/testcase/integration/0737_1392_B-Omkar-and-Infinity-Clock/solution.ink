// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  var i: dynamic;
  var n: dynamic;
  var k: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k);
    var a = cpp_array(n);
    var maxa: dynamic;
    {
      i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    if (((k % 2) != 0))
    {
      maxa = a[0];
      {
        i = 0;
        while ((i < n))
        {
          maxa = max(maxa, a[i]);
          i += 1;
        }
      }
      {
        i = 0;
        while ((i < n))
        {
          a[i] = (maxa - a[i]);
          i += 1;
        }
      }
    } else
    {
      maxa = a[0];
      {
        i = 0;
        while ((i < n))
        {
          maxa = max(maxa, a[i]);
          i += 1;
        }
      }
      {
        i = 0;
        while ((i < n))
        {
          a[i] = (maxa - a[i]);
          i += 1;
        }
      }
      maxa = a[0];
      {
        i = 0;
        while ((i < n))
        {
          maxa = max(maxa, a[i]);
          i += 1;
        }
      }
      {
        i = 0;
        while ((i < n))
        {
          a[i] = (maxa - a[i]);
          i += 1;
        }
      }
    }
    {
      i = 0;
      while ((i < n))
      {
        write(a[i], " ");
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
