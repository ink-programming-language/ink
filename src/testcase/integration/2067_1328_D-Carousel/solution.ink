// Translated from solution.cpp.

func read() -> dynamic
{
  var s: dynamic = 0;
  var w: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      w = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    s = ((((s << 3)) + ((s << 1))) + ((ch ^ 48)));
    ch = getchar();
  }
  return (s * w);
}

var mod: dynamic = 998244353;

var N: dynamic = 2000100;

var pi: dynamic = acos(-1);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

func main() -> dynamic
{
  var T: dynamic = read();
  while (cpp_update(T, "--"))
  {
    n = read();
    var cnt: dynamic = 1;
    var mark: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        a[i] = read();
        if ((i != 1))
        {
          if ((a[i] == a[(i - 1)]))
          {
            mark = 1;
            i += 1;
            continue;
          }
          cnt = 2;
        }
        i += 1;
      }
    }
    if ((cnt == 1))
    {
      printf("1\n");
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          printf("1 ");
          i += 1;
        }
      }
      printf("\n");
      continue;
    } else
    {
      if ((mark == 0))
      {
        if (((a[1] != a[n]) && ((n % 2) == 1)))
        {
          printf("3\n");
          {
            var i: dynamic = 1;
            while ((i <= (n - 1)))
            {
              if (((i % 2) == 1))
              {
                printf("1 ");
              } else
              {
                printf("2 ");
              }
              i += 1;
            }
          }
          printf("3");
          printf("\n");
          continue;
        } else
        {
          printf("2\n");
          {
            var i: dynamic = 1;
            while ((i <= n))
            {
              if (((i % 2) == 0))
              {
                printf("1 ");
              } else
              {
                printf("2 ");
              }
              i += 1;
            }
          }
          printf("\n");
          continue;
        }
      } else
      {
        if ((((n % 2) == 1) && (a[1] != a[n])))
        {
          printf("2\n");
          var loc: dynamic = 1;
          {
            var i: dynamic = 1;
            while ((i <= n))
            {
              if ((i == 1))
              {
                printf("%d ", loc);
                loc = 2;
              } else if ((mark == 1))
              {
                if ((a[i] == a[(i - 1)]))
                {
                  mark = 0;
                  if ((loc == 1))
                  {
                    printf("2 ");
                  } else
                  {
                    printf("1 ");
                  }
                } else
                {
                  printf("%d ", loc);
                  if ((loc == 2))
                  {
                    loc = 1;
                  } else
                  {
                    loc = 2;
                  }
                }
              } else
              {
                printf("%d ", loc);
                if ((loc == 2))
                {
                  loc = 1;
                } else
                {
                  loc = 2;
                }
              }
              i += 1;
            }
          }
          printf("\n");
        } else
        {
          printf("2\n");
          {
            var i: dynamic = 1;
            while ((i <= n))
            {
              if (((i % 2) == 0))
              {
                printf("1 ");
              } else
              {
                printf("2 ");
              }
              i += 1;
            }
          }
          printf("\n");
          continue;
        }
      }
    }
  }
}
