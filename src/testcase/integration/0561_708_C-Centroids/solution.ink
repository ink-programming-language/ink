// Translated from solution.cpp.

func read()
{
  var x = 0;
  var ch = getchar();
  var w = 1;
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

func write(x: dynamic)
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

func writeln(x: dynamic)
{
  write(x);
  puts("");
}

var n: dynamic;

var N = (420000 * 2);

class Edge
{
  var u: dynamic;
  var v: dynamic;
  var nxt: dynamic;
}

var e = cpp_array(N);

var head = cpp_array(N);

var en: dynamic;

func addl(x: dynamic, y: dynamic)
{
  e[cpp_update(en, "++")].u = x;
  e[en].v = y;
  e[en].nxt = head[x];
  head[x] = en;
}

var ans = cpp_array(N);

var siz = cpp_array(N);

var rt: dynamic;

var res = 1e9;

func dfs(x: dynamic, F: dynamic)
{
  siz[x] = 1;
  var mx = 0;
  {
    var i = head[x];
    while (i)
    {
      var y = e[i].v;
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

var sub: dynamic;

func solve(x: dynamic, F: dynamic, sum: dynamic, pre: dynamic)
{
  if ((sum <= (n / 2)))
  {
    ans[x] = 1;
  }
  {
    var i = 0;
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
    var i = head[x];
    while (i)
    {
      var y = e[i].v;
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

func main()
{
  n = read();
  {
    var i = 1;
    while ((i < n))
    {
      var x = read();
      var y = read();
      addl(x, y);
      addl(y, x);
      i += 1;
    }
  }
  dfs(1, 0);
  dfs(rt, 0);
  ans[rt] = 1;
  {
    var i = head[rt];
    while (i)
    {
      sub.push_back(make_pair(siz[e[i].v], e[i].v));
      i = e[i].nxt;
    }
  }
  sort(sub.begin(), sub.end(), greater());
  {
    var i = head[rt];
    while (i)
    {
      var to = e[i].v;
      solve(to, rt, (n - siz[to]), to);
      i = e[i].nxt;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  puts("");
  return 0;
}
