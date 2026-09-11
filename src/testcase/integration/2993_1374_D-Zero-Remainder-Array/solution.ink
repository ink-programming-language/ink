// Translated from solution.cpp.

var a: dynamic = cpp_array(200005);

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    read(n, k);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        var c: dynamic = cpp_uninitialized();
        read(c);
        a[i] = (c % k);
        i += 1;
      }
    }
    sort((a + 1), ((a + 1) + n));
    var x: dynamic = 0;
    var y: dynamic = 0;
    var cur: dynamic = 0;
    var tmp: dynamic = 0;
    {
      var i: dynamic = n;
      while ((i >= 1))
      {
        if ((a[i] != 0))
        {
          if ((a[i] == cur))
          {
            tmp += 1;
          } else
          {
            if ((tmp >= x))
            {
              x = tmp;
              y = cur;
            }
            cur = a[i];
            tmp = 1;
          }
        }
        i -= 1;
      }
    }
    if (((tmp >= x) && (cur != 0)))
    {
      x = tmp;
      y = cur;
    }
    if ((x == 0))
    {
      write(0, "\n");
    } else
    {
      write(((((x - 1)) * k) + (((k + 1) - y))), "\n");
    }
  }
  return 0;
}
