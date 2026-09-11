// Translated from solution.cpp.

func main() -> dynamic
{
  var nodes: dynamic = cpp_uninitialized();
  var edges: dynamic = cpp_uninitialized();
  read(nodes, edges);
  var adj: dynamic = cpp_array((nodes + 1));
  var visited: dynamic = cpp_construct((nodes + 1));
  var ans: dynamic = cpp_construct((nodes + 1));
  {
    var i: dynamic = 1;
    while ((i <= edges))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  q.push(1);
  ans[1] = 1;
  visited[1] = true;
  while ((!q.empty()))
  {
    var cur: dynamic = q.front();
    q.pop();
    for (var next: dynamic in adj[cur])
    {
      if ((!visited[next]))
      {
        q.push(next);
        visited[next] = true;
        ans[next] = cur;
      }
    }
  }
  write("Yes\n");
  {
    var i: dynamic = 2;
    while ((i <= nodes))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
}
