// Translated from solution.cpp.

var m: dynamic;

var n: dynamic;

var mn: dynamic;

var ans: dynamic;

var cnt: dynamic;

var p = cpp_array(4, 5005);

var c = cpp_array(5005);

var v = cpp_array(5005);

var vw: dynamic;

func f(c: dynamic)
{
  var __cpp_switch_1 = c;
  if (__cpp_switch_1 == cpp_char("L"))
  {
    return 0;
  }
  else if (__cpp_switch_1 == cpp_char("R"))
  {
    return 1;
  }
  else if (__cpp_switch_1 == cpp_char("U"))
  {
    return 2;
  }
  else if (__cpp_switch_1 == cpp_char("D"))
  {
    return 3;
  }
}

func solve(x: dynamic)
{
  {
    var i = 0;
    while ((i < mn))
    {
      p[i][0] = if (((i % n) == 0)) -1 else (i - 1);
      p[i][1] = if (((i % n) == (n - 1))) -1 else (i + 1);
      p[i][2] = (i - n);
      p[i][3] = (i + n);
      i += 1;
    }
  }
  vw = x;
  var ret = 0;
  while (1)
  {
    ret += 1;
    v[x] = vw;
    var z = f(c[x]);
    var X = x;
    while (((c[X] == cpp_char(".")) || (v[X] == vw)))
    {
      X = p[X][z];
      if ((!(((0 <= X) && (X < mn)))))
      {
        return ret;
      }
    }
    p[x][z] = X;
    x = X;
  }
}

func main()
{
  scanf("%d%d", (&m), (&n));
  mn = (m * n);
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%s", (c + (i * n)));
      i += 1;
    }
  }
  memset(v, -1, cpp_sizeof((v)));
  ans = cpp_assign(cnt, "=", 0);
  {
    var i = 0;
    while ((i < mn))
    {
      if ((c[i] != cpp_char(".")))
      {
        var tmp = solve(i);
        if ((tmp > ans))
        {
          ans = tmp;
          cnt = 1;
        } else if ((tmp == ans))
        {
          cnt += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d %d\n", ans, cnt);
  return 0;
}
