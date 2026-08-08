// Translated from solution.cpp.

var mod = 1000000007;

var INF = (mod * mod);

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func per(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=n-1;i>=0;i--)");
}

func rep1(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=1;i<=n;i++)");
}

func Rep(i: dynamic, sta: dynamic, n: dynamic)
{
  cpp_macro("for(int i=sta;i<n;i++)");
}

var stop = cpp_expression("#include<i");

class edge
{
  var to: dynamic;
  var cap: dynamic;
  var rev: dynamic;
}

var G = cpp_array(100000);

var used = cpp_array(100000);

var banned = cpp_array(100000);

func add_edge(from_cpp: dynamic, to: dynamic)
{
  G[from_cpp].push_back([to, 1, cpp_cast(G[to].size())]);
  G[to].push_back([from_cpp, 0, (cpp_cast(G[from_cpp].size()) - 1)]);
}

var l: dynamic;

var r: dynamic;

func dfs(v: dynamic, t: dynamic, f: dynamic)
{
  if ((v == t))
  {
    return f;
  }
  used[v] = true;
  {
    var i = 0;
    while ((i < cpp_cast(G[v].size())))
    {
      var e = G[v][i];
      if ((((!used[e.to]) && (!banned[e.to])) && (e.cap > 0)))
      {
        var d = dfs(e.to, t, min(f, e.cap));
        if ((d > 0))
        {
          e.cap -= d;
          G[e.to][e.rev].cap += d;
          return d;
        }
      }
      i += 1;
    }
  }
  return 0;
}

func max_flow(s: dynamic, t: dynamic)
{
  var flow = 0;
  {
    while (true)
    {
      memset(used, 0, cpp_sizeof((used)));
      var f = dfs(s, t, mod);
      if ((f == 0))
      {
        return flow;
      }
      flow += f;
    }
  }
}

var isodd = cpp_array(6000);

var flow: dynamic;

var rest: dynamic;

func del(x: dynamic)
{
  if (banned[x])
  {
    return;
  }
  banned[x] = true;
  rest -= 1;
  var nxt = -1;
  if (isodd[x])
  {
    for (var e in G[x])
    {
      if (((e.to == r) && (e.cap == 0)))
      {
        flow -= 1;
        e.cap = 1;
        G[e.to][e.rev].cap = 0;
      } else if (((e.to != r) && (e.cap == 1)))
      {
        e.cap = 0;
        G[e.to][e.rev].cap = 1;
        nxt = e.to;
      }
    }
    if ((nxt != -1))
    {
      for (var e in G[nxt])
      {
        if (((e.to == l) && (e.cap == 1)))
        {
          e.cap = 0;
          G[e.to][e.rev].cap = 1;
        }
      }
    }
  } else
  {
    for (var e in G[x])
    {
      if (((e.to == l) && (e.cap == 1)))
      {
        flow -= 1;
        e.cap = 0;
        G[e.to][e.rev].cap = 1;
      } else if (((e.to != l) && (e.cap == 0)))
      {
        e.cap = 1;
        G[e.to][e.rev].cap = 0;
        nxt = e.to;
      }
    }
    if ((nxt != -1))
    {
      for (var e in G[nxt])
      {
        if (((e.to == r) && (e.cap == 0)))
        {
          e.cap = 1;
          G[e.to][e.rev].cap = 0;
        }
      }
    }
  }
}

func add(x: dynamic)
{
  banned[x] = false;
  rest += 1;
}

func solve()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var x = cpp_array(6666);
  var y = cpp_array(6666);
  var odd: dynamic;
  var even: dynamic;
  l = n;
  r = (n + 1);
  rep(i, even.size());
  {
    add_edge(l, even[i]);
  }
  rep(j, odd.size());
  {
    add_edge(odd[j], r);
    isodd[odd[j]] = true;
  }
  rep(i, even.size());
  {
    rep(j, odd.size());
    {
      var dx = (x[even[i]] - x[odd[j]]);
      var dy = (y[even[i]] - y[odd[j]]);
      var dif = ((dx * dx) + (dy * dy));
      if ((dif < 4))
      {
        add_edge(even[i], odd[j]);
      }
    }
  }
  rest = n;
  flow = max_flow(l, r);
  if (((n - flow) < k))
  {
    write(-1, "\n");
    return;
  }
  var ans: dynamic;
  var cur = 0;
  var use = 0;
  while ((cur < n))
  {
    if (banned[cur])
    {
      cur += 1;
      continue;
    }
    use += 1;
    del(cur);
    var dels: dynamic;
    for (var e in G[cur])
    {
      if ((((e.to != l) && (e.to != r)) && (!banned[e.to])))
      {
        dels.push_back(e.to);
        del(e.to);
      }
    }
    flow += max_flow(l, r);
    if ((((rest - flow) + use) >= k))
    {
      ans.push_back(cur);
    } else
    {
      use -= 1;
      for (var e in dels)
      {
        add(e);
      }
      flow += max_flow(l, r);
    }
    cur += 1;
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  solve();
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    read(x[i], y[i]);
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((((x[i] % 2) == 1) && ((y[i] % 2) == 0)))
    {
      even.push_back(i);
    } else if ((((x[i] % 2) == 0) && ((y[i] % 2) == 0)))
    {
      even.push_back(i);
    } else
    {
      odd.push_back(i);
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    write((ans[i] + 1), "\n");
  }
