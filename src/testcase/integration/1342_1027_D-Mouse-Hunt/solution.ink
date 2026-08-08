// Translated from solution.cpp.

func prep()
{
  cin.tie(0);
  cin.sync_with_stdio(0);
}

func __cpp_top_level_1()
{
}

var mod = (cpp_cast(1e9) + 7);

var MX = (cpp_cast(2e5) + 20);

var mn = cpp_array(MX);

var g = cpp_array(MX);

var comp_g = cpp_array(MX);

var pre = cpp_array(MX);

var low = cpp_array(MX);

var id = cpp_array(MX);

var stk: dynamic;

var cntr: dynamic;

var scc: dynamic;

func tarjan(x: dynamic)
{
  if ((pre[x] != -1))
  {
    return;
  }
  stk.push(x);
  pre[x] = cpp_update(cntr, "++");
  low[x] = pre[x];
  for (var y in g[x])
  {
    if ((pre[y] == -1))
    {
      tarjan(y);
    }
    low[x] = min(low[x], low[y]);
  }
  if ((pre[x] == low[x]))
  {
    while (true)
    {
      var y = stk.top();
      stk.pop();
      low[y] = MX;
      id[y] = scc;
      if ((y == x))
      {
        break;
      }
    }
    scc += 1;
  }
}

func main()
{
  prep();
  var n: dynamic;
  read(n);
  var c = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(c[i]);
      mn[i] = cpp_cast(2e9);
      i += 1;
    }
  }
  {
    var i = 0;
    var x: dynamic;
    while ((i < n))
    {
      read(x);
      g[i].push_back(cpp_update(x, "--"));
      i += 1;
    }
  }
  memset(pre, 0xff, cpp_sizeof(pre));
  memset(id, 0xff, cpp_sizeof(id));
  {
    var i = 0;
    while ((i < n))
    {
      tarjan(i);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      mn[id[i]] = min(mn[id[i]], c[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      for (var x in g[i])
      {
        if ((id[x] != id[i]))
        {
          comp_g[id[i]].push_back(id[x]);
        }
      }
      i += 1;
    }
  }
  var res = 0;
  {
    var i = 0;
    while ((i < scc))
    {
      if ((comp_g[i].size() == 0))
      {
        res += mn[i];
      }
      i += 1;
    }
  }
  write(res, cpp_char("\n"));
  return 0;
}
