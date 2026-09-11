// Translated from solution.cpp.

var mtx1: dynamic = cpp_array(51, 51);

var n1: dynamic = cpp_uninitialized();

var m1: dynamic = cpp_uninitialized();

var mtx2: dynamic = cpp_array(51, 51);

var n2: dynamic = cpp_uninitialized();

var m2: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

func input() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n1), (&m1));
  {
    i = 1;
    while ((i <= n1))
    {
      scanf("%s", (mtx1[i] + 1));
      i += 1;
    }
  }
  scanf("%d%d", (&n2), (&m2));
  {
    i = 1;
    while ((i <= n2))
    {
      scanf("%s", (mtx2[i] + 1));
      i += 1;
    }
  }
}

func cnt(xi: dynamic, yi: dynamic) -> dynamic
{
  var res: dynamic = 0;
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 1;
    while (((i <= n1) && ((i + x) <= n2)))
    {
      if (((i + x) < 1))
      {
        i += 1;
        continue;
      }
      {
        j = 1;
        while (((j <= m1) && ((j + y) <= m2)))
        {
          if (((j + y) < 1))
          {
            j += 1;
            continue;
          }
          res += (((mtx1[i][j] - cpp_char("0"))) * ((mtx2[(i + x)][(j + y)] - cpp_char("0"))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return res;
}

func solv() -> dynamic
{
  ans = -1;
  {
    x = -50;
    while ((x < 51))
    {
      {
        y = -50;
        while ((y < 51))
        {
          var tmp: dynamic = cnt(x, y);
          if ((tmp > ans))
          {
            ans = tmp;
            X = x;
            Y = y;
          }
          y += 1;
        }
      }
      x += 1;
    }
  }
  printf("%d %d\n", X, Y);
}

func main() -> dynamic
{
  input();
  solv();
  return 0;
}
