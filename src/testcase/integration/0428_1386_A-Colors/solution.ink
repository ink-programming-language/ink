// Translated from solution.cpp.

var Inf: dynamic = 0x3f3f3f3f;

var INF: dynamic = 0x3f3f3f3f3f3f3f3f;

var inF: dynamic = 11451419198101145141919810.1145141919810;

var pi: dynamic = acosl(-1);

var n: dynamic = cpp_uninitialized();

func ask(x: dynamic) -> dynamic
{
  printf("? %lld\n", x);
  fflush(stdout);
  var res: dynamic = cpp_uninitialized();
  scanf("%d", (&res));
  return res;
}

func answer(x: dynamic) -> dynamic
{
  printf("= %lld\n", x);
  fflush(stdout);
}

func solve() -> dynamic
{
  scanf("%lld", (&n));
  var ps: dynamic = cpp_uninitialized();
  var l: dynamic = 1;
  var r: dynamic = (n - 1);
  while ((l <= r))
  {
    var m: dynamic = (((l + r)) >> 1);
    ps.push_back(m);
    if (((l == r) && (m == (n - 1))))
    {
      break;
    }
    l = (m + 1);
  }
  reverse((ps).begin(), (ps).end());
  var now: dynamic = n;
  var pre: dynamic = n;
  var tol: dynamic = 1;
  {
    typeof((ps).begin()) = (ps).begin();
    e_D = (ps).end();
    while ((i != e_D))
    {
      pre = now;
      if (tol)
      {
        now -= (*i);
      } else
      {
        now += (*i);
      }
      tol ^= 1;
      i += 1;
    }
  }
  if ((now > pre))
  {
    tol = 1;
  } else
  {
    tol = 0;
  }
  l = 1;
  r = (n - 1);
  var res: dynamic = n;
  ask(now);
  while ((r >= l))
  {
    var m: dynamic = (((l + r)) >> 1);
    if (tol)
    {
      now -= m;
    } else
    {
      now += m;
    }
    if (ask(now))
    {
      r = (m - 1);
      res = m;
    } else
    {
      l = (m + 1);
    }
    tol ^= 1;
  }
  answer(res);
}

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    solve();
  }
  return 0;
}
