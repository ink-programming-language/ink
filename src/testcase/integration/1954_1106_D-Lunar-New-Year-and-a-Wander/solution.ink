// Translated from solution.cpp.

var vis = cpp_array(100005);

var ans: dynamic;

var adj = cpp_array(100005);

func bfs_like_fn(n: dynamic)
{
  var pq: dynamic;
  pq.push(n);
  while ((!pq.empty()))
  {
    var fr = pq.top();
    pq.pop();
    if ((vis[fr] == true))
    {
      continue;
    }
    vis[fr] = true;
    ans.push_back(fr);
    {
      var i = 0;
      while ((i < adj[fr].size()))
      {
        if ((vis[adj[fr][i]] == false))
        {
          pq.push(adj[fr][i]);
        }
        i += 1;
      }
    }
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var x: dynamic;
  var y: dynamic;
  read(n, m);
  while (cpp_update(m, "--"))
  {
    read(x, y);
    adj[x].push_back(y);
    adj[y].push_back(x);
  }
  bfs_like_fn(1);
  for (var p in ans)
  {
    write(p, " ");
  }
  write("\n");
  return 0;
}
