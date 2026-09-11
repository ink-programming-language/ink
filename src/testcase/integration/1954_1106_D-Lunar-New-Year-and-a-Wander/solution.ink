// Translated from solution.cpp.

var vis: dynamic = cpp_array(100005);

var ans: dynamic = cpp_uninitialized();

var adj: dynamic = cpp_array(100005);

func bfs_like_fn(n: dynamic) -> dynamic
{
  var pq: dynamic = cpp_uninitialized();
  pq.push(n);
  while ((!pq.empty()))
  {
    var fr: dynamic = pq.top();
    pq.pop();
    if ((vis[fr] == true))
    {
      continue;
    }
    vis[fr] = true;
    ans.push_back(fr);
    {
      var i: dynamic = 0;
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(n, m);
  while (cpp_update(m, "--"))
  {
    read(x, y);
    adj[x].push_back(y);
    adj[y].push_back(x);
  }
  bfs_like_fn(1);
  for (var p: dynamic in ans)
  {
    write(p, " ");
  }
  write("\n");
  return 0;
}
