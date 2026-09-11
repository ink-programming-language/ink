// Translated from solution.cpp.

var N_max: dynamic = 0;

class graph
{
  var V: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_uninitialized();
  var L2: dynamic = cpp_uninitialized();
  func graph(n: dynamic) -> dynamic
  {
      V = n;
    }
  func add_edge(u: dynamic, v: dynamic) -> dynamic
  {
      L.push_back([u, v]);
    }
  func find_set(i: dynamic, parent: dynamic) -> dynamic
  {
      if ((parent[i] == -1))
      {
        return i;
      }
      parent[i] = find_set(parent[i], parent);
      return find_set(parent[i], parent);
    }
  func union_set(x: dynamic, y: dynamic, parent: dynamic, rank: dynamic) -> dynamic
  {
      var S1: dynamic = find_set(x, parent);
      var S2: dynamic = find_set(y, parent);
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
  func solve() -> dynamic
  {
      var m: dynamic = cpp_uninitialized();
      var i: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var ans: dynamic = cpp_uninitialized();
      var leader: dynamic = cpp_uninitialized();
      var sz: dynamic = cpp_uninitialized();
      var parent: dynamic = cpp_array(V);
      var rank: dynamic = cpp_array(V);
      var isValid: dynamic = cpp_array(V);
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
      for (var edge: dynamic in L)
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

func run_case() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var T: dynamic = 1;
  while (cpp_update(T, "--"))
  {
    run_case();
  }
  return 0;
}
