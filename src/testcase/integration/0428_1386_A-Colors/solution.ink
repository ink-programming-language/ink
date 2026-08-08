// Translated from solution.cpp.

var Inf = 0x3f3f3f3f;

var INF = 0x3f3f3f3f3f3f3f3f;

var inF = 11451419198101145141919810.1145141919810;

var pi = acosl(-1);

var n: dynamic;

func ask(x: dynamic)
{
  printf("? %lld\n", x);
  fflush(stdout);
  var res: dynamic;
  scanf("%d", (&res));
  return res;
}

func answer(x: dynamic)
{
  printf("= %lld\n", x);
  fflush(stdout);
}

func solve()
{
  scanf("%lld", (&n));
  var ps: dynamic;
  var l = 1;
  var r = (n - 1);
  while ((l <= r))
  {
    var m = (((l + r)) >> 1);
    ps.push_back(m);
    if (((l == r) && (m == (n - 1))))
    {
      break;
    }
    l = (m + 1);
  }
  reverse((ps).begin(), (ps).end());
  var now = n;
  var pre = n;
  var tol = 1;
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
  var res = n;
  ask(now);
  while ((r >= l))
  {
    var m = (((l + r)) >> 1);
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

func main()
{
  var T: dynamic;
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    solve();
  }
  return 0;
}
