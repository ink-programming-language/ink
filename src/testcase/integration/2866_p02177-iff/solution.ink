// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var G = cpp_array(305);

var revG = cpp_array(305);

var topo: dynamic;

var used = cpp_array(305);

var scc = cpp_array(305);

func tpsort(v: dynamic)
{
  used[v] = true;
  {
    var i = 0;
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

func sccdfs(v: dynamic, id: dynamic)
{
  used[v] = true;
  scc[v] = id;
  {
    var i = 0;
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

func main(argument_0: dynamic)
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  var u: dynamic;
  var v: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      read(u, v);
      G[u].push_back(v);
      revG[v].push_back(u);
      i += 1;
    }
  }
  {
    var i = 1;
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
  var id = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      used[i] = false;
      i += 1;
    }
  }
  {
    var i = 0;
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
    var i = 1;
    while ((i <= n))
    {
      var vec: dynamic;
      {
        var j = 1;
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
        var j = 0;
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
