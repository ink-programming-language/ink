// Translated from solution.cpp.

func sqr(x: dynamic)
{
  return (x * x);
}

var pi = 3.1415926535897932384626433832795;

var inf = cpp_cast(1e9);

var inf64 = cpp_cast(4e18);

var name = "b";

var NMAX = 10010;

var n: dynamic;

var num: dynamic;

var a = cpp_array(NMAX);

var ans: dynamic;

var zn = cpp_array(NMAX);

var NEXT = cpp_array(NMAX);

var st = cpp_array(NMAX);

var lst = cpp_array(NMAX);

var seg = cpp_array(NMAX);

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < cpp_cast(n)))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  ans = 0;
  {
    var i = 0;
    while ((i < cpp_cast(n)))
    {
      num = 0;
      memset(st, 255, cpp_sizeof((st)));
      memset(NEXT, 255, cpp_sizeof((NEXT)));
      {
        var j = 0;
        while ((j < cpp_cast(n)))
        {
          zn[num] = j;
          var tmp = max(0, (j - a[j]));
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
      var now = 0;
      var idx = 0;
      var last = -1;
      while ((idx < (n - 1)))
      {
        now += 1;
        var tmp = idx;
        {
          var j = (last + 1);
          while ((j <= tmp))
          {
            {
              var f = st[j];
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
        var j = (cpp_cast(n) - 1);
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
