// Translated from solution.cpp.

func main()
{
  var nodes: dynamic;
  var edges: dynamic;
  read(nodes, edges);
  var adj = cpp_array((nodes + 1));
  var visited = cpp_construct((nodes + 1));
  var ans = cpp_construct((nodes + 1));
  {
    var i = 1;
    while ((i <= edges))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  var q: dynamic;
  q.push(1);
  ans[1] = 1;
  visited[1] = true;
  while ((!q.empty()))
  {
    var cur = q.front();
    q.pop();
    for (var next in adj[cur])
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
    var i = 2;
    while ((i <= nodes))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
}
