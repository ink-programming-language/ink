// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(305);

var revG: dynamic = cpp_array(305);

var topo: dynamic = cpp_uninitialized();

var used: dynamic = cpp_array(305);

var scc: dynamic = cpp_array(305);

func tpsort(v: dynamic) -> dynamic
{
  used[v] = true;
  {
    var i: dynamic = 0;
    while ((i < G[v].size()))
    {
      if ((!used[G[v][i]]))
      {
        tpsort(G[v][i]);
      }
      i += 1;
    }
  }
  topo.push_back(v);
}

func sccdfs(v: dynamic, id: dynamic) -> dynamic
{
  used[v] = true;
  scc[v] = id;
  {
    var i: dynamic = 0;
    while ((i < revG[v].size()))
    {
      if ((!used[revG[v][i]]))
      {
        sccdfs(revG[v][i], id);
      }
      i += 1;
    }
  }
}

func main(argument_0: dynamic) -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(u, v);
      G[u].push_back(v);
      revG[v].push_back(u);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!used[i]))
      {
        tpsort(i);
      }
      i += 1;
    }
  }
  reverse(topo.begin(), topo.end());
  var id: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      used[i] = false;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < topo.size()))
    {
      if ((!used[topo[i]]))
      {
        sccdfs(topo[i], cpp_update(id, "++"));
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var vec: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((scc[i] == scc[j]))
          {
            vec.push_back(j);
          }
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < vec.size()))
        {
          write(vec[j]);
          if ((j < (cpp_cast(vec.size()) - 1)))
          {
            write(" ");
          }
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
