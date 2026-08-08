// Translated from solution.cpp.

var INF = 1e17;

var maxn = (2e5 + 700);

var mod = (1e9 + 7);

func read(a: dynamic)
{
  var c = getchar();
  var x = 0;
  var f = 1;
  while ((!isdigit(c)))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (isdigit(c))
  {
    x = (((((x << 1)) + ((x << 3))) + c) - cpp_char("0"));
    c = getchar();
  }
  a = (f * x);
}

var n: dynamic;

var m: dynamic;

var p: dynamic;

var s = cpp_array(maxn);

var a = cpp_array(maxn);

var b = cpp_array(maxn);

func digit(x: dynamic, d: dynamic)
{
  var ans = 0;
  while (x)
  {
    d[cpp_update(ans, "++")] = (x % 10);
    x /= 10;
  }
  return ans;
}

var path = cpp_array(maxn);

func check(pos: dynamic, lbound: dynamic, rbound: dynamic)
{
  if ((!pos))
  {
    return 1;
  }
  if (((!lbound) && (!rbound)))
  {
    return 1;
  }
  var l = if (lbound) a[pos] else 0;
  var r = if (rbound) b[pos] else 9;
  {
    var i = l;
    while ((i <= r))
    {
      if ((path[i] > 0))
      {
        path[i] -= 1;
        if (check((pos - 1), (lbound && (i == l)), (rbound && (i == r))))
        {
          path[i] += 1;
          return 1;
        }
        path[i] += 1;
      }
      i += 1;
    }
  }
  return 0;
}

var pos: dynamic;

var res = 0;

func dfs(u: dynamic, w: dynamic)
{
  if ((u == 9))
  {
    path[u] = w;
    if (check(pos, 1, 1))
    {
      res += 1;
    }
    return;
  }
  {
    var i = 0;
    while ((i <= w))
    {
      path[u] = i;
      dfs((u + 1), (w - i));
      i += 1;
    }
  }
}

func main()
{
  read(n);
  read(m);
  pos = digit(n, a);
  pos = digit(m, b);
  dfs(0, pos);
  printf("%d\n", res);
  return 0;
}
