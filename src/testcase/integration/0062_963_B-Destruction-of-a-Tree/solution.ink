// Translated from solution.cpp.

var maxn: dynamic = ((cpp_cast(2e5)) + 5);

var adj: dynamic = cpp_array(maxn);

var del: dynamic = cpp_array(maxn);

var INDEX: dynamic = 0;

var onStack: dynamic = cpp_array(maxn);

var index: dynamic = cpp_array(maxn);

var lowLink: dynamic = cpp_array(maxn);

var stk: dynamic = cpp_uninitialized();

var components: dynamic = cpp_uninitialized();

func getConnectedComponent(i: dynamic) -> dynamic
{
  index[i] = INDEX;
  lowLink[i] = INDEX;
  INDEX += 1;
  stk.push(i);
  onStack[i] = true;
  for (var nxt: dynamic in adj[i])
  {
    if ((index[nxt] == -1))
    {
      getConnectedComponent(nxt);
      lowLink[i] = min(lowLink[i], lowLink[nxt]);
    } else if (onStack[nxt])
    {
      lowLink[i] = min(lowLink[i], index[nxt]);
    }
  }
  if ((lowLink[i] == index[i]))
  {
    var comp: dynamic = cpp_uninitialized();
    var poped: dynamic = cpp_uninitialized();
    while (true)
    {
      poped = stk.top();
      stk.pop();
      onStack[poped] = false;
      comp.push_back(poped);
      if (!(((poped != i))))
      {
        break;
      }
    }
    if ((comp.size() > 0))
    {
      components.push_back(comp);
    }
  }
}

func tarjan(n: dynamic) -> dynamic
{
  memset(onStack, false, cpp_sizeof((onStack)));
  fill(index, (index + n), -1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((index[i] == -1))
      {
        getConnectedComponent(i);
      }
      i += 1;
    }
  }
}

func dfs(curr: dynamic, par: dynamic = -1) -> dynamic
{
  var mustDel: dynamic = 0;
  for (var nxt: dynamic in del[curr])
  {
    if ((nxt != par))
    {
      mustDel += dfs(nxt, curr);
    }
  }
  if ((par == -1))
  {
    if ((mustDel & 1))
    {
      return 1;
    } else
    {
      return 0;
    }
  }
  if (((mustDel % 2) == 0))
  {
    adj[par].push_back(curr);
    return 1;
  } else
  {
    adj[curr].push_back(par);
    return 0;
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      x -= 1;
      if ((x == -1))
      {
        i += 1;
        continue;
      }
      del[x].push_back((i - 1));
      del[(i - 1)].push_back(x);
      i += 1;
    }
  }
  if ((dfs(0) == 1))
  {
    write("NO");
  } else
  {
    write("YES\n");
    tarjan(n);
    reverse(components.begin(), components.end());
    for (var comp: dynamic in components)
    {
      write((comp[0] + 1), "\n");
    }
    tarjan(n);
  }
}
