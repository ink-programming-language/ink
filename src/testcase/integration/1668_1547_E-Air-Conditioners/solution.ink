// Translated from solution.cpp.

var q: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(300010);

var t: dynamic = cpp_array(300010);

var l: dynamic = cpp_array(300010);

var r: dynamic = cpp_array(300010);

var p: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(q);
  while (cpp_update(q, "--"))
  {
    memset(t, 0x3f, cpp_sizeof((t)));
    read(n, k);
    {
      var i: dynamic = 1;
      while ((i <= k))
      {
        read(a[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= k))
      {
        read(t[a[i]]);
        i += 1;
      }
    }
    p = 0x3f3f3f3f;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        p = min((p + 1), t[i]);
        l[i] = p;
        i += 1;
      }
    }
    p = 0x3f3f3f3f;
    {
      var i: dynamic = n;
      while ((i >= 1))
      {
        p = min((p + 1), t[i]);
        r[i] = p;
        i -= 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        write(min(l[i], r[i]), " ");
        i += 1;
      }
    }
    write("\n");
  }
}
