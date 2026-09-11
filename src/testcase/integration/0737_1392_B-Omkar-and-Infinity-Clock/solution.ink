// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k);
    var a: dynamic = cpp_array(n);
    var maxa: dynamic = cpp_uninitialized();
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
