// Translated from solution.cpp.

var INF: dynamic = 1e17;

var maxn: dynamic = (2e5 + 700);

var mod: dynamic = (1e9 + 7);

func read(a: dynamic) -> dynamic
{
  var c: dynamic = getchar();
  var x: dynamic = 0;
  var f: dynamic = 1;
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

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(maxn);

var a: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

func digit(x: dynamic, d: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  while (x)
  {
    d[cpp_update(ans, "++")] = (x % 10);
    x /= 10;
  }
  return ans;
}

var path: dynamic = cpp_array(maxn);

func check(pos: dynamic, lbound: dynamic, rbound: dynamic) -> dynamic
{
  if ((!pos))
  {
    return 1;
  }
  if (((!lbound) && (!rbound)))
  {
    return 1;
  }
  var l: dynamic =  (lbound) ? a[pos] : 0;
  var r: dynamic =  (rbound) ? b[pos] : 9;
  {
    var i: dynamic = l;
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

var pos: dynamic = cpp_uninitialized();

var res: dynamic = 0;

func dfs(u: dynamic, w: dynamic) -> dynamic
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
    var i: dynamic = 0;
    while ((i <= w))
    {
      path[u] = i;
      dfs((u + 1), (w - i));
      i += 1;
    }
  }
}

func main() -> dynamic
{
  read(n);
  read(m);
  pos = digit(n, a);
  pos = digit(m, b);
  dfs(0, pos);
  printf("%d\n", res);
  return 0;
}
