// Translated from solution.cpp.

var N: dynamic = 1010;

class Edge
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((N * 2));

var lst: dynamic = cpp_array(N);

var d: dynamic = cpp_uninitialized();

func add(x: dynamic, y: dynamic) -> dynamic
{
  e[d].to = y;
  e[d].next = lst[x];
  lst[x] = cpp_update(d, "++");
}

var fa: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var sz: dynamic = cpp_array(N);

func read() -> dynamic
{
  var w: dynamic = 0;
  var f: dynamic = 0;
  var c: dynamic = getchar();
  while (((((c < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = 1;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    w = (((w * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return  (f) ? (-w) : w;
}

func dfs1(t: dynamic) -> dynamic
{
  sz[t] = 1;
  {
    var i: dynamic = lst[t];
    while ((i >= 0))
    {
      if ((e[i].to != fa[t]))
      {
        fa[e[i].to] = t;
        dfs1(e[i].to);
        sz[t] += sz[e[i].to];
      }
      i = e[i].next;
    }
  }
}

func dfs2(t: dynamic, num: dynamic, mul: dynamic) -> dynamic
{
  var tmp: dynamic = 0;
  {
    var i: dynamic = lst[t];
    while ((i >= 0))
    {
      if ((e[i].to != fa[t]))
      {
        printf("%d %d %d\n", t, e[i].to, (((tmp + 1)) * mul));
        dfs2(e[i].to, (sz[e[i].to] - 1), mul);
        tmp += sz[e[i].to];
      }
      i = e[i].next;
    }
  }
}

var Sz: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  n = read();
  if ((n == 1))
  {
    return 0;
  }
  memset(lst, -1, cpp_sizeof(lst));
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      x = read();
      y = read();
      add(x, y);
      add(y, x);
      i += 1;
    }
  }
  var Lim: dynamic = (((n + 1)) / 3);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          fa[j] = 0;
          j += 1;
        }
      }
      fa[i] = i;
      dfs1(i);
      Sz.clear();
      {
        var j: dynamic = lst[i];
        while ((j >= 0))
        {
          Sz.push_back(make_pair(sz[e[j].to], e[j].to));
          j = e[j].next;
        }
      }
      sort(Sz.begin(), Sz.end());
      var sm: dynamic = 0;
      var now: dynamic = 0;
      {
        now = 0;
        while ((now < Sz.size()))
        {
          sm += Sz[now].first;
          if (((sm * ((n - sm))) >= (((2 * n) * n) / 9)))
          {
            break;
          }
          now += 1;
        }
      }
      if (((sm * ((n - sm))) < (((2 * n) * n) / 9)))
      {
        i += 1;
        continue;
      }
      var tmp: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j <= now))
        {
          printf("%d %d %d\n", i, Sz[j].second, (tmp + 1));
          dfs2(Sz[j].second, (Sz[j].first - 1), 1);
          tmp = (tmp + Sz[j].first);
          j += 1;
        }
      }
      tmp = 0;
      {
        var j: dynamic = (now + 1);
        while ((j < Sz.size()))
        {
          printf("%d %d %d\n", i, Sz[j].second, (((tmp + 1)) * ((sm + 1))));
          dfs2(Sz[j].second, (Sz[j].first - 1), (sm + 1));
          tmp = (tmp + Sz[j].first);
          j += 1;
        }
      }
      break;
      i += 1;
    }
  }
  return 0;
}
