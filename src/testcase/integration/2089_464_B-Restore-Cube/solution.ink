// Translated from solution.cpp.

func Abs(first: dynamic)
{
  return (if ((first < 0)) (-first) else first);
}

func Sqr(first: dynamic)
{
  return ((first * first));
}

func plural(s: dynamic)
{
  return (if ((int_cpp((s).size()) && (s[(int_cpp((s).size()) - 1)] == cpp_char("x")))) (s + "en") else (s + "s"));
}

var INF = cpp_cast(1e9);

var EPS = 1e-12;

var PI = acos(-1.0);

func Read(first: dynamic)
{
  var c: dynamic;
  var r = 0;
  var n = 0;
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

var done: dynamic;

var ord = cpp_array(8);

var V = cpp_array(3, 8);

var cur = cpp_array(3, 8);

var ans = cpp_array(3, 8);

func dist(first: dynamic, second: dynamic, z: dynamic)
{
  return (((Sqr(first) + Sqr(second)) + Sqr(z)));
}

func test()
{
  var i: dynamic;
  var j: dynamic;
  var c1: dynamic;
  var c2: dynamic;
  var c3: dynamic;
  var m: dynamic;
  var d = cpp_array(8, 8);
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

func rec(i: dynamic)
{
  var j: dynamic;
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

func main()
{
  if (0)
  {
    freopen("in.txt", "r", stdin);
  }
  var i: dynamic;
  var j: dynamic;
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
            printf("%d%c", ans[i][j], if ((j == 2)) cpp_char("\n") else cpp_char(" "));
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
  return (0);
}
