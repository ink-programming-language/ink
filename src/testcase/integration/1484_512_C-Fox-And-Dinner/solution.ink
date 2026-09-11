// Translated from solution.cpp.

var inf: dynamic = cpp_cast(1e9);

var INF: dynamic = cpp_cast(5e18);

var MOD: dynamic = 998244353;

func abs(x: dynamic) -> dynamic
{
  return  ((x < 0)) ? (-x) : x;
}

func add(x: dynamic, y: dynamic) -> dynamic
{
  x += y;
  return  ((x >= MOD)) ? (x - MOD) : x;
}

func sub(x: dynamic, y: dynamic) -> dynamic
{
  x -= y;
  return  ((x < 0)) ? (x + MOD) : x;
}

func Add(x: dynamic, y: dynamic) -> dynamic
{
  x += y;
  if ((x >= MOD))
  {
    x -= MOD;
  }
}

func Sub(x: dynamic, y: dynamic) -> dynamic
{
  x -= y;
  if ((x < 0))
  {
    x += MOD;
  }
}

func Mul(x: dynamic, y: dynamic) -> dynamic
{
  x = ((cpp_cast((x)) * (y)) % MOD);
}

func qpow(x: dynamic, y: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  while (y)
  {
    if ((y & 1))
    {
      ret = ((cpp_cast((ret)) * (x)) % MOD);
    }
    x = ((cpp_cast((x)) * (x)) % MOD);
    y >>= 1;
  }
  return ret;
}

func checkmin(x: dynamic, y: dynamic) -> dynamic
{
  if ((x > y))
  {
    x = y;
  }
}

func checkmax(x: dynamic, y: dynamic) -> dynamic
{
  if ((x < y))
  {
    x = y;
  }
}

func checkmin(x: dynamic, y: dynamic) -> dynamic
{
  if ((x > y))
  {
    x = y;
  }
}

func checkmax(x: dynamic, y: dynamic) -> dynamic
{
  if ((x < y))
  {
    x = y;
  }
}

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  while (((c > cpp_char("9")) || (c < cpp_char("0"))))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
    c = getchar();
  }
  return (x * f);
}

var N: dynamic = 1001;

var M: dynamic = 500005;

var first: dynamic = cpp_array(N);

var nxt: dynamic = cpp_array(M);

var point: dynamic = cpp_array(M);

var w: dynamic = cpp_array(M);

var cur: dynamic = cpp_array(N);

var e: dynamic = 0;

var tot: dynamic = 0;

var S: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var id: dynamic = cpp_array(N, N);

func add_edge(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  point[e] = y;
  w[e] = z;
  nxt[e] = first[x];
  first[x] = cpp_update(e, "++");
}

func add(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  add_edge(x, y, z);
  add_edge(y, x, 0);
  id[x][y] = (e - 1);
  id[y][x] = (e - 1);
}

var vis: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

func bfs() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  while ((!q.empty()))
  {
    q.pop();
  }
  {
    var i: dynamic = 1;
    while ((i <= tot))
    {
      vis[i] = 0;
      i += 1;
    }
  }
  vis[S] = 1;
  q.push(S);
  dep[S] = 0;
  while ((!q.empty()))
  {
    var u: dynamic = q.front();
    q.pop();
    {
      var i: dynamic = first[u];
      while ((i != -1))
      {
        if (w[i])
        {
          var to: dynamic = point[i];
          if (vis[to])
          {
            i = nxt[i];
            continue;
          }
          dep[to] = (dep[u] + 1);
          vis[to] = 1;
          q.push(to);
        }
        i = nxt[i];
      }
    }
  }
  return vis[T];
}

func dfs(u: dynamic, flow: dynamic) -> dynamic
{
  if ((u == T))
  {
    return flow;
  }
  var ret: dynamic = flow;
  {
    var i: dynamic = cur[u];
    while ((i != -1))
    {
      if (w[i])
      {
        var to: dynamic = point[i];
        if ((dep[to] != (dep[u] + 1)))
        {
          i = nxt[i];
          continue;
        }
        var tmp: dynamic = dfs(to, min(w[i], ret));
        if (tmp)
        {
          w[i] -= tmp;
          w[(i ^ 1)] += tmp;
          ret -= tmp;
        }
        if ((!ret))
        {
          break;
        }
      }
      i = nxt[i];
    }
  }
  return (flow - ret);
}

func Dinic() -> dynamic
{
  var ret: dynamic = 0;
  while (bfs())
  {
    {
      var i: dynamic = 1;
      while ((i <= tot))
      {
        cur[i] = first[i];
        i += 1;
      }
    }
    ret += dfs(S, inf);
  }
  return ret;
}

var n: dynamic = cpp_uninitialized();

var bl: dynamic = cpp_array(M);

func init() -> dynamic
{
  memset(first, -1, cpp_sizeof((first)));
  n = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i < M))
    {
      if ((!bl[i]))
      {
        {
          var j: dynamic = (i + i);
          while ((j < M))
          {
            bl[j] = 1;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  tot = n;
  S = cpp_update(tot, "++");
  T = cpp_update(tot, "++");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((a[i] & 1))
      {
        Flow.add(S, i, 2);
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            if ((!bl[(a[i] + a[j])]))
            {
              Flow.add(i, j, 1);
            }
            j += 1;
          }
        }
      } else
      {
        Flow.add(i, T, 2);
      }
      i += 1;
    }
  }
}

var vis: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  init();
  var F: dynamic = Flow.Dinic();
  if ((F != n))
  {
    puts("Impossible");
    return 0;
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        var v: dynamic = cpp_uninitialized();
        var x: dynamic = i;
        while (1)
        {
          vis[x] = 1;
          v.push_back(x);
          var to: dynamic = -1;
          {
            var j: dynamic = 1;
            while ((j <= n))
            {
              if ((w[id[x][j]] && (!vis[j])))
              {
                to = j;
                break;
              }
              j += 1;
            }
          }
          if ((to == -1))
          {
            break;
          }
          x = to;
        }
        ans.push_back(v);
      }
      i += 1;
    }
  }
  printf("%d\n", cpp_cast((ans).size()));
  for (var v: dynamic in ans)
  {
    printf("%d ", cpp_cast((v).size()));
    for (var u: dynamic in v)
    {
      printf("%d ", u);
    }
    puts("");
  }
  return 0;
}
