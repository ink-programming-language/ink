// Translated from solution.cpp.

func sqr(x: dynamic) -> dynamic
{
  return (x * x);
}

var pi: dynamic = 3.1415926535897932384626433832795;

var inf: dynamic = cpp_cast(1e9);

var inf64: dynamic = cpp_cast(4e18);

var name: dynamic = "b";

var NMAX: dynamic = 10010;

var n: dynamic = cpp_uninitialized();

var num: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(NMAX);

var ans: dynamic = cpp_uninitialized();

var zn: dynamic = cpp_array(NMAX);

var NEXT: dynamic = cpp_array(NMAX);

var st: dynamic = cpp_array(NMAX);

var lst: dynamic = cpp_array(NMAX);

var seg: dynamic = cpp_array(NMAX);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(n)))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  ans = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(n)))
    {
      num = 0;
      memset(st, 255, cpp_sizeof((st)));
      memset(NEXT, 255, cpp_sizeof((NEXT)));
      {
        var j: dynamic = 0;
        while ((j < cpp_cast(n)))
        {
          zn[num] = j;
          var tmp: dynamic = max(0, (j - a[j]));
          if ((st[tmp] == -1))
          {
            st[tmp] = num;
            lst[tmp] = num;
          } else
          {
            NEXT[lst[tmp]] = num;
            lst[tmp] = num;
          }
          num += 1;
          j += 1;
        }
      }
      var now: dynamic = 0;
      var idx: dynamic = 0;
      var last: dynamic = -1;
      while ((idx < (n - 1)))
      {
        now += 1;
        var tmp: dynamic = idx;
        {
          var j: dynamic = (last + 1);
          while ((j <= tmp))
          {
            {
              var f: dynamic = st[j];
              while ((f != -1))
              {
                idx = max(idx, zn[f]);
                f = NEXT[f];
              }
            }
            j += 1;
          }
        }
        last = tmp;
      }
      ans += now;
      {
        var j: dynamic = (cpp_cast(n) - 1);
        while ((j >= 0))
        {
          a[(j + 1)] = a[j];
          j -= 1;
        }
      }
      a[0] = a[n];
      i += 1;
    }
  }
  write(ans, "\n");
  write(clock(), "\n");
  return 0;
}
