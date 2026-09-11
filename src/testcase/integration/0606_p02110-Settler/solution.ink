// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var INF: dynamic = (mod * mod);

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func per(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=n-1;i>=0;i--)");
}

func rep1(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=n;i++)");
}

func Rep(i: dynamic, sta: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=sta;i<n;i++)");
}

var stop: dynamic = cpp_expression("#include<i");

class edge
{
  var to: dynamic = cpp_uninitialized();
  var cap: dynamic = cpp_uninitialized();
  var rev: dynamic = cpp_uninitialized();
}

var G: dynamic = cpp_array(100000);

var used: dynamic = cpp_array(100000);

var banned: dynamic = cpp_array(100000);

func add_edge(from_cpp: dynamic, to: dynamic) -> dynamic
{
  G[from_cpp].push_back([to, 1, cpp_cast(G[to].size())]);
  G[to].push_back([from_cpp, 0, (cpp_cast(G[from_cpp].size()) - 1)]);
}

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

func dfs(v: dynamic, t: dynamic, f: dynamic) -> dynamic
{
  if ((v == t))
  {
    return f;
  }
  used[v] = true;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(G[v].size())))
    {
      var e: dynamic = G[v][i];
      if ((((!used[e.to]) && (!banned[e.to])) && (e.cap > 0)))
      {
        var d: dynamic = dfs(e.to, t, min(f, e.cap));
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

func max_flow(s: dynamic, t: dynamic) -> dynamic
{
  var flow: dynamic = 0;
  {
    while (true)
    {
      memset(used, 0, cpp_sizeof((used)));
      var f: dynamic = dfs(s, t, mod);
      if ((f == 0))
      {
        return flow;
      }
      flow += f;
    }
  }
}

var isodd: dynamic = cpp_array(6000);

var flow: dynamic = cpp_uninitialized();

var rest: dynamic = cpp_uninitialized();

func del(x: dynamic) -> dynamic
{
  if (banned[x])
  {
    return;
  }
  banned[x] = true;
  rest -= 1;
  var nxt: dynamic = -1;
  if (isodd[x])
  {
    for (var e: dynamic in G[x])
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
      for (var e: dynamic in G[nxt])
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
    for (var e: dynamic in G[x])
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
      for (var e: dynamic in G[nxt])
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

func add(x: dynamic) -> dynamic
{
  banned[x] = false;
  rest += 1;
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var x: dynamic = cpp_array(6666);
  var y: dynamic = cpp_array(6666);
  var odd: dynamic = cpp_uninitialized();
  var even: dynamic = cpp_uninitialized();
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
      var dx: dynamic = (x[even[i]] - x[odd[j]]);
      var dy: dynamic = (y[even[i]] - y[odd[j]]);
      var dif: dynamic = ((dx * dx) + (dy * dy));
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
  var ans: dynamic = cpp_uninitialized();
  var cur: dynamic = 0;
  var use: dynamic = 0;
  while ((cur < n))
  {
    if (banned[cur])
    {
      cur += 1;
      continue;
    }
    use += 1;
    del(cur);
    var dels: dynamic = cpp_uninitialized();
    for (var e: dynamic in G[cur])
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
      for (var e: dynamic in dels)
      {
        add(e);
      }
      flow += max_flow(l, r);
    }
    cur += 1;
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  solve();
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(x[i], y[i]);
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    write((ans[i] + 1), "\n");
  }
