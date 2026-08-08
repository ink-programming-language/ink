// Translated from solution.cpp.

func read()
{
  var s = 0;
  var w = 1;
  var ch = getchar();
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

var mod = 998244353;

var N = 2000100;

var pi = acos(-1);

var n: dynamic;

var m: dynamic;

var s: dynamic;

var t: dynamic;

var w: dynamic;

var a = cpp_array(N);

func main()
{
  var T = read();
  while (cpp_update(T, "--"))
  {
    n = read();
    var cnt = 1;
    var mark = 0;
    {
      var i = 1;
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
        var i = 1;
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
            var i = 1;
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
            var i = 1;
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
          var loc = 1;
          {
            var i = 1;
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
            var i = 1;
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
