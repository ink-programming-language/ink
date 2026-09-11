// Translated from solution.cpp.

var maxn: dynamic = 123;

var a: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

var vis: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

class P
{
  var a: dynamic = cpp_array(101);
  func operator_less(t: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((a[i] != t.a[i]))
          {
            return (a[i] < t.a[i]);
          }
          i += 1;
        }
      }
      return false;
    }
}

var t: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  var tmp: dynamic = (((1 << 15)) - 1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (a + i));
      b[i] = (a[i] >> 15);
      a[i] = (a[i] & tmp);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= tmp))
    {
      var mx: dynamic = 34;
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          t.a[j] = builtin_popcount((a[j] ^ i));
          mx = min(mx, t.a[j]);
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          t.a[j] -= mx;
          j += 1;
        }
      }
      m[t] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= tmp))
    {
      var mx: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          t.a[j] = builtin_popcount((b[j] ^ i));
          mx = max(mx, t.a[j]);
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          t.a[j] = (mx - t.a[j]);
          j += 1;
        }
      }
      if ((m.count(t) != 0))
      {
        return (0 * printf("%d", (m[t] + ((i << 15)))));
      }
      i += 1;
    }
  }
  printf("-1");
}
