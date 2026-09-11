// Translated from solution.cpp.

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var mn: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var ind: dynamic = cpp_uninitialized();

var inx: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var a: dynamic = cpp_array((n + 5));
    {
      i = 0;
      while ((i < n))
      {
        read(a[i].first, a[i].second);
        i += 1;
      }
    }
    sort(a, (a + n));
    var f: dynamic = cpp_array(1005);
    memset(f, 0, cpp_sizeof(f));
    var s: dynamic = "";
    ind = 0;
    inx = 0;
    mx = 0;
    var fl: dynamic = 1;
    {
      i = 0;
      while ((i < n))
      {
        x = a[i].first;
        y = a[i].second;
        if ((y < mx))
        {
          fl = 0;
          break;
        }
        if (((i > 0) && (a[i].first != a[(i - 1)].first)))
        {
          {
            j = ind;
            while ((j < mx))
            {
              s += "U";
              j += 1;
            }
          }
          ind = mx;
        }
        if ((f[x] == 0))
        {
          {
            j = inx;
            while ((j < x))
            {
              s += "R";
              j += 1;
            }
          }
          inx = x;
        }
        f[x] = 1;
        mx = max(mx, y);
        i += 1;
      }
    }
    {
      j = ind;
      while ((j < mx))
      {
        s += "U";
        j += 1;
      }
    }
    if ((fl == 0))
    {
      write("NO\n");
    } else
    {
      write("YES\n");
      write(s, "\n");
    }
  }
}
