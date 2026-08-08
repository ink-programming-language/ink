// Translated from solution.cpp.

class debugger
{
  func operator(v: dynamic)
  {
      write(v, " ");
      return (*this);
    }
}

var dbg: dynamic;

var N = (100000 + 10);

class data
{
  var to: dynamic;
  var next: dynamic;
}

var tu = cpp_array((N * 2));

var head = cpp_array(N);

var ip: dynamic;

var dfn = cpp_array(N);

var low = cpp_array(N);

var sccno = cpp_array(N);

var step: dynamic;

var scc_cnt: dynamic;

func init()
{
  ip = 0;
  memset(head, -1, cpp_sizeof((head)));
}

func add(u: dynamic, v: dynamic)
{
  tu[ip].to = v;
  tu[ip].next = head[u];
  head[u] = cpp_update(ip, "++");
}

var scc = cpp_array(N);

var S: dynamic;

func dfs(u: dynamic)
{
  dfn[u] = cpp_assign(low[u], "=", cpp_update(step, "++"));
  S.push(u);
  {
    var i = head[u];
    while ((i != -1))
    {
      var v = tu[i].to;
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
      var x = S.top();
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

func tarjan(n: dynamic)
{
  memset(sccno, 0, cpp_sizeof((sccno)));
  memset(dfn, 0, cpp_sizeof((dfn)));
  step = cpp_assign(scc_cnt, "=", 0);
  {
    var i = 1;
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

var u = cpp_array((100000 + 10));

var ou = cpp_array((100000 + 10));

func main()
{
  init();
  var n: dynamic;
  var m: dynamic;
  var h: dynamic;
  scanf("%d%d%d", (&n), (&m), (&h));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&u[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var a: dynamic;
      var b: dynamic;
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
    var i = 1;
    while ((i <= n))
    {
      {
        var j = head[i];
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
  var ans = 0;
  {
    var i = 1;
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
  for (var i in scc[ans])
  {
    printf("%d ", i);
  }
  return 0;
}
