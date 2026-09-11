// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var ch: dynamic = getchar();
  var w: dynamic = 1;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      w = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * w);
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  if ((x > 9))
  {
    write((x / 10));
  }
  putchar(((x % 10) + cpp_char("0")));
}

func writeln(x: dynamic) -> dynamic
{
  write(x);
  puts("");
}

var n: dynamic = cpp_uninitialized();

var N: dynamic = (420000 * 2);

class Edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var nxt: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(N);

var head: dynamic = cpp_array(N);

var en: dynamic = cpp_uninitialized();

func addl(x: dynamic, y: dynamic) -> dynamic
{
  e[cpp_update(en, "++")].u = x;
  e[en].v = y;
  e[en].nxt = head[x];
  head[x] = en;
}

var ans: dynamic = cpp_array(N);

var siz: dynamic = cpp_array(N);

var rt: dynamic = cpp_uninitialized();

var res: dynamic = 1e9;

func dfs(x: dynamic, F: dynamic) -> dynamic
{
  siz[x] = 1;
  var mx: dynamic = 0;
  {
    var i: dynamic = head[x];
    while (i)
    {
      var y: dynamic = e[i].v;
      if ((y == F))
      {
        i = e[i].nxt;
        continue;
      }
      dfs(y, x);
      siz[x] += siz[y];
      mx = max(mx, siz[y]);
      i = e[i].nxt;
    }
  }
  mx = max(mx, (n - siz[x]));
  if ((mx < res))
  {
    res = mx;
    rt = x;
  }
}

var sub: dynamic = cpp_uninitialized();

func solve(x: dynamic, F: dynamic, sum: dynamic, pre: dynamic) -> dynamic
{
  if ((sum <= (n / 2)))
  {
    ans[x] = 1;
  }
  {
    var i: dynamic = 0;
    while (((i < 2) && (i < sub.size())))
    {
      if ((sub[i].second == pre))
      {
        i += 1;
        continue;
      }
      if ((((n - siz[x]) - sub[i].first) <= (n / 2)))
      {
        ans[x] = 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = head[x];
    while (i)
    {
      var y: dynamic = e[i].v;
      if ((y == F))
      {
        i = e[i].nxt;
        continue;
      }
      solve(y, x, sum, pre);
      i = e[i].nxt;
    }
  }
}

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var x: dynamic = read();
      var y: dynamic = read();
      addl(x, y);
      addl(y, x);
      i += 1;
    }
  }
  dfs(1, 0);
  dfs(rt, 0);
  ans[rt] = 1;
  {
    var i: dynamic = head[rt];
    while (i)
    {
      sub.push_back(make_pair(siz[e[i].v], e[i].v));
      i = e[i].nxt;
    }
  }
  sort(sub.begin(), sub.end(), greater());
  {
    var i: dynamic = head[rt];
    while (i)
    {
      var to: dynamic = e[i].v;
      solve(to, rt, (n - siz[to]), to);
      i = e[i].nxt;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  puts("");
  return 0;
}
