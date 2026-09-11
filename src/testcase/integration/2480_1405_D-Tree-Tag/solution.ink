// Translated from solution.cpp.

class itr
{
  var begin: dynamic = cpp_uninitialized();
  var end: dynamic = cpp_uninitialized();
}

func get_range(b: dynamic, e: dynamic) -> dynamic
{
  return [b, e];
}

func __cpp_top_level_1() -> dynamic
{
}

class debug
{
  func operator_shift_left(argument_0: dynamic) -> dynamic
  {
      return (*self);
    }
}

func ARR(arr: dynamic, sz: dynamic) -> dynamic
{
  var ret: dynamic = ("{ " + to_string(arr[0]));
  {
    var i: dynamic = 1;
    while ((i < sz))
    {
      ret += (" , " + to_string(arr[i]));
      i += 1;
    }
  }
  ret += " }";
  return ret;
}

var INF: dynamic = (1e9 + 7);

var MxN: dynamic = (1e5 + 100);

var adj: dynamic = cpp_array(MxN);

var d: dynamic = cpp_array(MxN);

var dmx: dynamic = cpp_uninitialized();

var vmx: dynamic = cpp_uninitialized();

func dfs(u: dynamic, p: dynamic) -> dynamic
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
  for (var v: dynamic in adj[u])
  {
    if ((v != p))
    {
      dfs(v, u);
    }
  }
  return;
}

func main(argument_0: dynamic) -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var TC: dynamic = cpp_uninitialized();
  read(TC);
  while (cpp_update(TC, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var da: dynamic = cpp_uninitialized();
    var db: dynamic = cpp_uninitialized();
    read(n, a, b, da, db);
    {
      var i: dynamic = 0;
      while ((i <= n))
      {
        adj[i].clear();
        i += 1;
      }
    }
    a -= 1;
    b -= 1;
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        var u: dynamic = cpp_uninitialized();
        var v: dynamic = cpp_uninitialized();
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
    var dist: dynamic = (d[b]);
    dmx = 0;
    d[vmx] = 0;
    dfs(vmx, vmx);
    var ok: dynamic = 1;
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
    write(( (ok) ? "Bob\n" : "Alice\n"));
  }
  return 0;
}
