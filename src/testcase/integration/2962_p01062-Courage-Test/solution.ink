// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var pp: dynamic = cpp_uninitialized();

func dfs(u: dynamic, p: dynamic, G: dynamic, num: dynamic) -> dynamic
{
  num[u] = 1;
  {
    var i: dynamic = 0;
    while ((i < G[u].size()))
    {
      var v: dynamic = G[u][i];
      if ((v == p))
      {
        i += 1;
        continue;
      }
      dfs(v, u, G, num);
      num[u] += num[v];
      if ((num[v] == (N / 2)))
      {
        pp = i_i(u, v);
      }
      i += 1;
    }
  }
}

class union_find
{
  var v: dynamic = cpp_uninitialized();
  func union_find(n: dynamic) -> dynamic
  {
      self->v = cpp_construct(n, -1);
    }
  func find(x: dynamic) -> dynamic
  {
      return  (((v[x] < 0))) ? x : (cpp_assign(v[x], "=", find(v[x])));
    }
  func unite(x: dynamic, y: dynamic) -> dynamic
  {
      x = find(x);
      y = find(y);
      if ((x != y))
      {
        if (((-v[x]) < (-v[y])))
        {
          swap(x, y);
        }
        v[x] += v[y];
        v[y] = x;
      }
    }
  func same(x: dynamic, y: dynamic) -> dynamic
  {
      return (find(x) == find(y));
    }
  func size(x: dynamic) -> dynamic
  {
      return (-v[find(x)]);
    }
}

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(N, x, y);
  x -= 1;
  y -= 1;
  if ((N == 2))
  {
    write("Yes", "\n");
    return 0;
  }
  if (((N % 2) == 1))
  {
    write("No", "\n");
    return 0;
  }
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (N - 1)))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      a -= 1;
      b -= 1;
      G[a].push_back(b);
      G[b].push_back(a);
      d[a] += 1;
      d[b] += 1;
      A.push_back(a);
      B.push_back(b);
      i += 1;
    }
  }
  dfs(0, -1, G, num);
  {
    var i: dynamic = 0;
    while ((i < (N - 1)))
    {
      var a: dynamic = A[i];
      var b: dynamic = B[i];
      if (((i_i(a, b) == pp) || (i_i(b, a) == pp)))
      {
        d[a] -= 1;
        d[b] -= 1;
      } else
      {
        uf.unite(a, b);
      }
      i += 1;
    }
  }
  {
    var u: dynamic = 0;
    while ((u < N))
    {
      if ((d[u] >= 3))
      {
        write("No", "\n");
        return 0;
      }
      u += 1;
    }
  }
  if (uf.same(x, y))
  {
    write("No", "\n");
    return 0;
  }
  if (((d[x] == 1) && (d[y] == 1)))
  {
    write("Yes", "\n");
    return 0;
  }
  write("No", "\n");
}
