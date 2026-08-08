// Translated from solution.cpp.

var q: dynamic;

var n: dynamic;

var k: dynamic;

var a = cpp_array(300010);

var t = cpp_array(300010);

var l = cpp_array(300010);

var r = cpp_array(300010);

var p: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  read(q);
  while (cpp_update(q, "--"))
  {
    memset(t, 0x3f, cpp_sizeof((t)));
    read(n, k);
    {
      var i = 1;
      while ((i <= k))
      {
        read(a[i]);
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= k))
      {
        read(t[a[i]]);
        i += 1;
      }
    }
    p = 0x3f3f3f3f;
    {
      var i = 1;
      while ((i <= n))
      {
        p = min((p + 1), t[i]);
        l[i] = p;
        i += 1;
      }
    }
    p = 0x3f3f3f3f;
    {
      var i = n;
      while ((i >= 1))
      {
        p = min((p + 1), t[i]);
        r[i] = p;
        i -= 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        write(min(l[i], r[i]), " ");
        i += 1;
      }
    }
    write("\n");
  }
}
