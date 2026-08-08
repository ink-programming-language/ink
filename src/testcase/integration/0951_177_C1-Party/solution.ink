// Translated from solution.cpp.

var N_max = 0;

class graph
{
  var V: dynamic;
  var k: dynamic;
  var m: dynamic;
  var L: dynamic;
  var L2: dynamic;
  func graph(n: dynamic)
  {
      V = n;
    }
  func add_edge(u: dynamic, v: dynamic)
  {
      L.push_back([u, v]);
    }
  func find_set(i: dynamic, parent: dynamic)
  {
      if ((parent[i] == -1))
      {
        return i;
      }
      parent[i] = find_set(parent[i], parent);
      return find_set(parent[i], parent);
    }
  func union_set(x: dynamic, y: dynamic, parent: dynamic, rank: dynamic)
  {
      var S1 = find_set(x, parent);
      var S2 = find_set(y, parent);
      if ((S1 != S2))
      {
        if ((rank[S1] < rank[S2]))
        {
          parent[S1] = S2;
          rank[S2] += rank[S1];
        } else
        {
          parent[S2] = S1;
          rank[S1] += rank[S2];
        }
      }
    }
  func solve()
  {
      var m: dynamic;
      var i: dynamic;
      var x: dynamic;
      var y: dynamic;
      var u: dynamic;
      var v: dynamic;
      var ans: dynamic;
      var leader: dynamic;
      var sz: dynamic;
      var parent = cpp_array(V);
      var rank = cpp_array(V);
      var isValid = cpp_array(V);
      {
        i = 0;
        while ((i < V))
        {
          parent[i] = -1;
          rank[i] = 1;
          isValid[i] = true;
          i += 1;
        }
      }
      for (var edge in L)
      {
        u = edge.first;
        v = edge.second;
        union_set(u, v, parent, rank);
      }
      read(m);
      {
        i = 0;
        while ((i < m))
        {
          read(u, v);
          u -= 1;
          v -= 1;
          x = find_set(u, parent);
          y = find_set(v, parent);
          if ((x == y))
          {
            isValid[x] = false;
          }
          i += 1;
        }
      }
      ans = 0;
      {
        i = 0;
        while ((i < V))
        {
          leader = find_set(i, parent);
          if (isValid[leader])
          {
            sz = rank[leader];
            ans = max(ans, sz);
          }
          i += 1;
        }
      }
      write(ans, "\n");
    }
}

func run_case()
{
  var n: dynamic;
  var i: dynamic;
  var k: dynamic;
  var u: dynamic;
  var v: dynamic;
  read(n);
  read(k);
  {
    i = 0;
    while ((i < k))
    {
      read(u, v);
      u -= 1;
      v -= 1;
      G.add_edge(u, v);
      i += 1;
    }
  }
  G.solve();
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var T = 1;
  while (cpp_update(T, "--"))
  {
    run_case();
  }
  return 0;
}
