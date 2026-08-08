// Translated from solution.cpp.

class itr
{
  var begin: dynamic;
  var end: dynamic;
}

func get_range(b: dynamic, e: dynamic)
{
  return [b, e];
}

func __cpp_top_level_1()
{
}

class debug
{
  func operator_shift_left(argument_0: dynamic)
  {
      return (*this);
    }
}

func ARR(arr: dynamic, sz: dynamic)
{
  var ret = ("{ " + to_string(arr[0]));
  {
    var i = 1;
    while ((i < sz))
    {
      ret += (" , " + to_string(arr[i]));
      i += 1;
    }
  }
  ret += " }";
  return ret;
}

var INF = (1e9 + 7);

var MxN = (1e5 + 100);

var adj = cpp_array(MxN);

var d = cpp_array(MxN);

var dmx: dynamic;

var vmx: dynamic;

func dfs(u: dynamic, p: dynamic)
{
  if ((p != u))
  {
    d[u] = (d[p] + 1);
  }
  if ((d[u] > dmx))
  {
    dmx = d[u];
    vmx = u;
  }
  for (var v in adj[u])
  {
    if ((v != p))
    {
      dfs(v, u);
    }
  }
  return;
}

func main(argument_0: dynamic)
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var TC: dynamic;
  read(TC);
  while (cpp_update(TC, "--"))
  {
    var n: dynamic;
    var a: dynamic;
    var b: dynamic;
    var da: dynamic;
    var db: dynamic;
    read(n, a, b, da, db);
    {
      var i = 0;
      while ((i <= n))
      {
        adj[i].clear();
        i += 1;
      }
    }
    a -= 1;
    b -= 1;
    {
      var i = 1;
      while ((i < n))
      {
        var u: dynamic;
        var v: dynamic;
        read(u, v);
        u -= 1;
        v -= 1;
        adj[u].push_back(v);
        adj[v].push_back(u);
        i += 1;
      }
    }
    dmx = 0;
    d[a] = 0;
    dfs(a, a);
    var dist = (d[b]);
    dmx = 0;
    d[vmx] = 0;
    dfs(vmx, vmx);
    var ok = 1;
    if ((dist <= da))
    {
      ok = 0;
    } else if (((2 * da) >= dmx))
    {
      ok = 0;
    } else if ((db > (2 * da)))
    {
      ok = 1;
    } else if ((db <= (2 * da)))
    {
      ok = 0;
    }
    write((if (ok) "Bob\n" else "Alice\n"));
  }
  return 0;
}
