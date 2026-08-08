// Translated from solution.cpp.

func show(a: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func show(a: dynamic, r: dynamic, l: dynamic)
{
  {
    var i = 0;
    while ((i < r))
    {
      show(a[i], l);
      i += 1;
    }
  }
  write("\n");
}

var N = 310;

var M = 5000;

var oo = ((10000 * 10000) * 10);

var n: dynamic;

var m: dynamic;

var nth: dynamic;

var g = cpp_array(N, N);

func go(x: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  {
    i = (1 + x);
    while (((i + x) <= n))
    {
      {
        j = (1 + x);
        while (((j + x) <= m))
        {
          if ((((((g[i][j] == cpp_char("*")) && (g[(i - x)][j] == cpp_char("*"))) && (g[(i + x)][j] == cpp_char("*"))) && (g[i][(j + x)] == cpp_char("*"))) && (g[i][(j - x)] == cpp_char("*"))))
          {
            nth -= 1;
            if ((nth == 0))
            {
              printf("%d %d\n", i, j);
              printf("%d %d\n", (i - x), j);
              printf("%d %d\n", (i + x), j);
              printf("%d %d\n", i, (j - x));
              printf("%d %d\n", i, (j + x));
              return true;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return false;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var cas = 0;
  scanf("%d %d %d", (&n), (&m), (&nth));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%s", (g[i] + 1));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if (go(i))
      {
        return 0;
      }
      i += 1;
    }
  }
  puts("-1");
  return 0;
}
