// Translated from solution.cpp.

func show(a: dynamic, n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func show(a: dynamic, r: dynamic, l: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < r))
    {
      show(a[i], l);
      i += 1;
    }
  }
  write("\n");
}

var N: dynamic = 310;

var M: dynamic = 5000;

var oo: dynamic = ((10000 * 10000) * 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var nth: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(N, N);

func go(x: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var cas: dynamic = 0;
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
