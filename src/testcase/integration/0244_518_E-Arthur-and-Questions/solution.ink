// Translated from solution.cpp.

var a: dynamic = cpp_array(220020);

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var st: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(220020);

var s: dynamic = cpp_array(100);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%s", s);
      if ((s[0] == cpp_char("?")))
      {
        f[i] = 0;
      } else
      {
        f[i] = 1;
        sscanf(s, "%d", (&a[i]));
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      a[(i + n)] = 1020000000;
      f[(i + n)] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      l = -1020000000;
      t = 0;
      {
        var j: dynamic = i;
        while ((j <= (n + k)))
        {
          if ((!f[j]))
          {
            t += 1;
          } else
          {
            if ((t >= (a[j] - l)))
            {
              printf("Incorrect sequence\n");
              return 0;
            }
            r = a[j];
            if (((min((-l), r) * 2) > t))
            {
              st = (-((t / 2)));
            } else if ((((-l)) < r))
            {
              st = (l + 1);
            } else
            {
              st = (r - t);
            }
            {
              var p: dynamic = t;
              while ((p > 0))
              {
                a[(j - (p * k))] = cpp_update(st, "++");
                p -= 1;
              }
            }
            t = 0;
            l = r;
          }
          j += k;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      printf("%d ", a[i]);
      i += 1;
    }
  }
  printf("%d\n", a[n]);
  return 0;
}
