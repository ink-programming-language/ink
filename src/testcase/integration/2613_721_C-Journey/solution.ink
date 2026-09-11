// Translated from solution.cpp.

func split(s: dynamic, c: dynamic) -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  while (getline(ss, x, c))
  {
    v.push_back(move(x));
  }
  return v;
}

func err(it: dynamic) -> dynamic
{
}

func err(it: dynamic, a: dynamic, args: dynamic...) -> dynamic
{
  write(it->substr((((*it))[0] == cpp_char(" ")), it->length()), " = ", a, cpp_char("\n"));
  err(cpp_update(it, "++"), cpp_expand(args));
}

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var adj: dynamic = cpp_array(5013);

var dist: dynamic = cpp_array(5013, 5013);

var visited: dynamic = cpp_array(5013, 5013);

var prevn: dynamic = cpp_array(5013, 5013);

var pq: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  read(N, M, T);
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      read(u, v, t);
      adj[(u - 1)][(v - 1)] = t;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = 0;
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
    var p: dynamic = pq.top();
    pq.pop();
    var d: dynamic = p.first;
    var v: dynamic = p.second.first;
    var n: dynamic = p.second.second;
    if (visited[v][n])
    {
      continue;
    }
    visited[v][n] = true;
    for (var p: dynamic in adj[v])
    {
      var u: dynamic = p.first;
      if (((d + adj[v][u]) < dist[u][(n + 1)]))
      {
        dist[u][(n + 1)] = (d + adj[v][u]);
        pq.push([dist[u][(n + 1)], [u, (n + 1)]]);
        prevn[u][(n + 1)] = v;
      }
    }
  }
  var k: dynamic = N;
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
  var c: dynamic = (N - 1);
  var ans: dynamic = cpp_uninitialized();
  ans.push_back(c);
  while ((prevn[c][k] != -1))
  {
    c = prevn[c][k];
    ans.push_back(c);
    k -= 1;
  }
  {
    var i: dynamic = (ans.size() - 1);
    while ((i >= 0))
    {
      write((ans[i] + 1), cpp_char(" "));
      i -= 1;
    }
  }
  write("\n");
  return 0;
}
