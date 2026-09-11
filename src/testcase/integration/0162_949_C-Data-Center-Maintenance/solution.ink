// Translated from solution.cpp.

class debugger
{
  func operator(v: dynamic) -> dynamic
  {
      write(v, " ");
      return (*self);
    }
}

var dbg: dynamic = cpp_uninitialized();

var N: dynamic = (100000 + 10);

class data
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var tu: dynamic = cpp_array((N * 2));

var head: dynamic = cpp_array(N);

var ip: dynamic = cpp_uninitialized();

var dfn: dynamic = cpp_array(N);

var low: dynamic = cpp_array(N);

var sccno: dynamic = cpp_array(N);

var step: dynamic = cpp_uninitialized();

var scc_cnt: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  ip = 0;
  memset(head, -1, cpp_sizeof((head)));
}

func add(u: dynamic, v: dynamic) -> dynamic
{
  tu[ip].to = v;
  tu[ip].next = head[u];
  head[u] = cpp_update(ip, "++");
}

var scc: dynamic = cpp_array(N);

var S: dynamic = cpp_uninitialized();

func dfs(u: dynamic) -> dynamic
{
  dfn[u] = cpp_assign(low[u], "=", cpp_update(step, "++"));
  S.push(u);
  {
    var i: dynamic = head[u];
    while ((i != -1))
    {
      var v: dynamic = tu[i].to;
      if ((!dfn[v]))
      {
        dfs(v);
        low[u] = min(low[u], low[v]);
      } else if ((!sccno[v]))
      {
        low[u] = min(low[u], dfn[v]);
      }
      i = tu[i].next;
    }
  }
  if ((low[u] == dfn[u]))
  {
    scc_cnt += 1;
    scc[scc_cnt].clear();
    while (1)
    {
      var x: dynamic = S.top();
      S.pop();
      if ((sccno[x] != scc_cnt))
      {
        scc[scc_cnt].push_back(x);
      }
      sccno[x] = scc_cnt;
      if ((x == u))
      {
        break;
      }
    }
  }
}

func tarjan(n: dynamic) -> dynamic
{
  memset(sccno, 0, cpp_sizeof((sccno)));
  memset(dfn, 0, cpp_sizeof((dfn)));
  step = cpp_assign(scc_cnt, "=", 0);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!dfn[i]))
      {
        dfs(i);
      }
      i += 1;
    }
  }
}

var u: dynamic = cpp_array((100000 + 10));

var ou: dynamic = cpp_array((100000 + 10));

func main() -> dynamic
{
  init();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&m), (&h));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&u[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      if (((((u[a] + 1)) % (h)) == u[b]))
      {
        add(a, b);
      }
      if (((((u[b] + 1)) % (h)) == u[a]))
      {
        add(b, a);
      }
      i += 1;
    }
  }
  tarjan(n);
  memset(ou, true, cpp_sizeof((ou)));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = head[i];
        while ((j != -1))
        {
          if ((sccno[i] != sccno[tu[j].to]))
          {
            ou[sccno[i]] = false;
          }
          j = tu[j].next;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= scc_cnt))
    {
      if (ou[i])
      {
        if (((ans == 0) || (scc[ans].size() > scc[i].size())))
        {
          ans = i;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", int_cpp(scc[ans].size()));
  for (var i: dynamic in scc[ans])
  {
    printf("%d ", i);
  }
  return 0;
}
