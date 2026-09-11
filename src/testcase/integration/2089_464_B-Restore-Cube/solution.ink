// Translated from solution.cpp.

func Abs(first: dynamic) -> dynamic
{
  return ( ((first < 0)) ? (-first) : first);
}

func Sqr(first: dynamic) -> dynamic
{
  return ((first * first));
}

func plural(s: dynamic) -> dynamic
{
  return ( ((int_cpp((s).size()) && (s[(int_cpp((s).size()) - 1)] == cpp_char("x")))) ? (s + "en") : (s + "s"));
}

var INF: dynamic = cpp_cast(1e9);

var EPS: dynamic = 1e-12;

var PI: dynamic = acos(-1.0);

func Read(first: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  var r: dynamic = 0;
  var n: dynamic = 0;
  first = 0;
  {
    while (true)
    {
      c = getchar();
      if ((((c < 0)) && ((!r))))
      {
        return (0);
      }
      if ((((c == cpp_char("-"))) && ((!r))))
      {
        n = 1;
      } else if ((((c >= cpp_char("0"))) && ((c <= cpp_char("9")))))
      {
        first = (((first * 10) + c) - cpp_char("0"));
        r = 1;
      } else if (r)
      {
        break;
      }
    }
  }
  if (n)
  {
    first = (-first);
  }
  return (1);
}

var done: dynamic = cpp_uninitialized();

var ord: dynamic = cpp_array(8);

var V: dynamic = cpp_array(3, 8);

var cur: dynamic = cpp_array(3, 8);

var ans: dynamic = cpp_array(3, 8);

func dist(first: dynamic, second: dynamic, z: dynamic) -> dynamic
{
  return (((Sqr(first) + Sqr(second)) + Sqr(z)));
}

func test() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var c1: dynamic = cpp_uninitialized();
  var c2: dynamic = cpp_uninitialized();
  var c3: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_array(8, 8);
  m = (cpp_cast(INF) * INF);
  {
    i = 0;
    while ((i < 8))
    {
      {
        j = 0;
        while ((j < i))
        {
          d[i][j] = cpp_assign(d[j][i], "=", dist((cur[i][0] - cur[j][0]), (cur[i][1] - cur[j][1]), (cur[i][2] - cur[j][2])));
          m = min(m, d[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((!m))
  {
    return (0);
  }
  {
    i = 0;
    while ((i < 8))
    {
      c1 = cpp_assign(c2, "=", cpp_assign(c3, "=", 0));
      {
        j = 0;
        while ((j < 8))
        {
          if ((i != j))
          {
            if ((d[i][j] == m))
            {
              c1 += 1;
            } else if ((d[i][j] == (m * 2)))
            {
              c2 += 1;
            } else if ((d[i][j] == (m * 3)))
            {
              c3 += 1;
            } else
            {
              return (0);
            }
          }
          j += 1;
        }
      }
      if (((((c1 != 3)) || ((c2 != 3))) || ((c3 != 1))))
      {
        return (0);
      }
      i += 1;
    }
  }
  return (1);
}

func rec(i: dynamic) -> dynamic
{
  var j: dynamic = cpp_uninitialized();
  if ((i == 8))
  {
    {
      j = 0;
      while ((j < 8))
      {
        if ((ord[j] == 0))
        {
          cur[j][0] = V[j][0];
          cur[j][1] = V[j][1];
          cur[j][2] = V[j][2];
        }
        if ((ord[j] == 1))
        {
          cur[j][0] = V[j][0];
          cur[j][1] = V[j][2];
          cur[j][2] = V[j][1];
        }
        if ((ord[j] == 2))
        {
          cur[j][0] = V[j][1];
          cur[j][1] = V[j][0];
          cur[j][2] = V[j][2];
        }
        if ((ord[j] == 3))
        {
          cur[j][0] = V[j][1];
          cur[j][1] = V[j][2];
          cur[j][2] = V[j][0];
        }
        if ((ord[j] == 4))
        {
          cur[j][0] = V[j][2];
          cur[j][1] = V[j][0];
          cur[j][2] = V[j][1];
        }
        if ((ord[j] == 5))
        {
          cur[j][0] = V[j][2];
          cur[j][1] = V[j][1];
          cur[j][2] = V[j][0];
        }
        j += 1;
      }
    }
    if (test())
    {
      done = 1;
      memcpy(ans, cur, cpp_sizeof((cur)));
    }
    return;
  }
  {
    j = 0;
    while ((j < 6))
    {
      ord[i] = j;
      rec((i + 1));
      if (done)
      {
        break;
      }
      j += 1;
    }
  }
}

func main() -> dynamic
{
  if (0)
  {
    freopen("in.txt", "r", stdin);
  }
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < 8))
    {
      {
        j = 0;
        while ((j < 3))
        {
          Read(V[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  rec(1);
  if ((!done))
  {
    printf("NO\n");
  } else
  {
    printf("YES\n");
    {
      i = 0;
      while ((i < 8))
      {
        {
          j = 0;
          while ((j < 3))
          {
            printf("%d%c", ans[i][j],  ((j == 2)) ? cpp_char("\n") : cpp_char(" "));
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
  return (0);
}
