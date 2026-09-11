// Translated from solution.cpp.

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var mn: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(4, 5005);

var c: dynamic = cpp_array(5005);

var v: dynamic = cpp_array(5005);

var vw: dynamic = cpp_uninitialized();

func f(c: dynamic) -> dynamic
{
  var __cpp_switch_1: dynamic = c;
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

func solve(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < mn))
    {
      p[i][0] =  (((i % n) == 0)) ? -1 : (i - 1);
      p[i][1] =  (((i % n) == (n - 1))) ? -1 : (i + 1);
      p[i][2] = (i - n);
      p[i][3] = (i + n);
      i += 1;
    }
  }
  vw = x;
  var ret: dynamic = 0;
  while (1)
  {
    ret += 1;
    v[x] = vw;
    var z: dynamic = f(c[x]);
    var X: dynamic = x;
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

func main() -> dynamic
{
  scanf("%d%d", (&m), (&n));
  mn = (m * n);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%s", (c + (i * n)));
      i += 1;
    }
  }
  memset(v, -1, cpp_sizeof((v)));
  ans = cpp_assign(cnt, "=", 0);
  {
    var i: dynamic = 0;
    while ((i < mn))
    {
      if ((c[i] != cpp_char(".")))
      {
        var tmp: dynamic = solve(i);
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
