// Translated from solution.cpp.

func split(s: dynamic, c: dynamic)
{
  var v: dynamic;
  var x: dynamic;
  while (getline(ss, x, c))
  {
    v.push_back(move(x));
  }
  return v;
}

func err(it: dynamic)
{
}

func err(it: dynamic, a: dynamic, args: dynamic...)
{
  write(it->substr((((*it))[0] == cpp_char(" ")), it->length()), " = ", a, cpp_char("\n"));
  err(cpp_update(it, "++"), cpp_expand(args));
}

var N: dynamic;

var M: dynamic;

var T: dynamic;

var adj = cpp_array(5013);

var dist = cpp_array(5013, 5013);

var visited = cpp_array(5013, 5013);

var prevn = cpp_array(5013, 5013);

var pq: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  read(N, M, T);
  {
    var i = 0;
    while ((i < M))
    {
      var u: dynamic;
      var v: dynamic;
      var t: dynamic;
      read(u, v, t);
      adj[(u - 1)][(v - 1)] = t;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while ((j <= N))
        {
          dist[i][j] = 1000000013;
          prevn[i][j] = -1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  dist[0][1] = 0;
  pq.push([0, [0, 1]]);
  while ((!pq.empty()))
  {
    var p = pq.top();
    pq.pop();
    var d = p.first;
    var v = p.second.first;
    var n = p.second.second;
    if (visited[v][n])
    {
      continue;
    }
    visited[v][n] = true;
    for (var p in adj[v])
    {
      var u = p.first;
      if (((d + adj[v][u]) < dist[u][(n + 1)]))
      {
        dist[u][(n + 1)] = (d + adj[v][u]);
        pq.push([dist[u][(n + 1)], [u, (n + 1)]]);
        prevn[u][(n + 1)] = v;
      }
    }
  }
  var k = N;
  while (k)
  {
    if ((dist[(N - 1)][k] <= T))
    {
      break;
    }
    k -= 1;
  }
  if ((!k))
  {
    write(-1, "\n");
    return 0;
  }
  write(k, cpp_char("\n"));
  var c = (N - 1);
  var ans: dynamic;
  ans.push_back(c);
  while ((prevn[c][k] != -1))
  {
    c = prevn[c][k];
    ans.push_back(c);
    k -= 1;
  }
  {
    var i = (ans.size() - 1);
    while ((i >= 0))
    {
      write((ans[i] + 1), cpp_char(" "));
      i -= 1;
    }
  }
  write("\n");
  return 0;
}
