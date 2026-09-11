// Translated from solution.cpp.

var inf: dynamic = 1e9;

class Matching
{
  var n: dynamic = cpp_uninitialized();
  var matchL: dynamic = cpp_uninitialized();
  var matchR: dynamic = cpp_uninitialized();
  var dist: dynamic = cpp_uninitialized();
  var seen: dynamic = cpp_uninitialized();
  var ke: dynamic = cpp_uninitialized();
  func Matching(n: dynamic) -> dynamic
  {
      self->n = cpp_construct(n);
      self->matchL = cpp_construct((n + 1));
      self->matchR = cpp_construct((n + 1));
      self->dist = cpp_construct((n + 1));
      self->seen = cpp_construct((n + 1), false);
      self->ke = cpp_construct((n + 1));
    }
  func addEdge(u: dynamic, v: dynamic) -> dynamic
  {
      ke[u].push_back(v);
    }
  func bfs() -> dynamic
  {
      var qu: dynamic = cpp_uninitialized();
      {
        var u: dynamic = 1;
        while ((u <= n))
        {
          if ((!matchL[u]))
          {
            dist[u] = 0;
            qu.push(u);
          } else
          {
            dist[u] = inf;
          }
          u += 1;
        }
      }
      dist[0] = inf;
      while ((!qu.empty()))
      {
        var u: dynamic = qu.front();
        qu.pop();
        {
          typeof(ke[u].begin()) = ke[u].begin();
          while ((v != ke[u].end()))
          {
            if ((dist[matchR[(*v)]] == inf))
            {
              dist[matchR[(*v)]] = (dist[u] + 1);
              qu.push(matchR[(*v)]);
            }
            v += 1;
          }
        }
      }
      return (dist[0] != inf);
    }
  func dfs(u: dynamic) -> dynamic
  {
      if (u)
      {
        {
          typeof(ke[u].begin()) = ke[u].begin();
          while ((v != ke[u].end()))
          {
            if (((dist[matchR[(*v)]] == (dist[u] + 1)) && dfs(matchR[(*v)])))
            {
              matchL[u] = (*v);
              matchR[(*v)] = u;
              return true;
            }
            v += 1;
          }
        }
        dist[u] = inf;
        return false;
      }
      return true;
    }
  func match_cpp() -> dynamic
  {
      var res: dynamic = 0;
      while (bfs())
      {
        {
          var u: dynamic = 1;
          while ((u <= n))
          {
            if ((!matchL[u]))
            {
              if (dfs(u))
              {
                res += 1;
              }
            }
            u += 1;
          }
        }
      }
      return res;
    }
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var from_cpp: dynamic = cpp_array(605);

var to: dynamic = cpp_array(605);

var need: dynamic = cpp_array(605);

var sneed: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(605);

func main(argument_0: dynamic) -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var ntest: dynamic = cpp_uninitialized();
  read(ntest);
  while (cpp_update(ntest, "--"))
  {
    read(n, m, k);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        need[i] = 0;
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        read(from_cpp[i], to[i]);
        need[from_cpp[i]] += 1;
        need[to[i]] += 1;
        i += 1;
      }
    }
    sneed = 0;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        need[i] = (2 * max(0, (need[i] - k)));
        sneed += need[i];
        i += 1;
      }
    }
    if ((sneed > m))
    {
      {
        var i: dynamic = 1;
        while ((i <= m))
        {
          write(0, cpp_char(" "));
          i += 1;
        }
      }
      write(cpp_char("\n"));
      continue;
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        need[i] += need[(i - 1)];
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        var u: dynamic = from_cpp[i];
        var v: dynamic = to[i];
        {
          var j: dynamic = (need[(u - 1)] + 1);
          while ((j <= need[u]))
          {
            G.addEdge(i, j);
            j += 1;
          }
        }
        {
          var j: dynamic = (need[(v - 1)] + 1);
          while ((j <= need[v]))
          {
            G.addEdge(i, j);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var cnt: dynamic = G.match_cpp();
    if ((cnt < sneed))
    {
      {
        var i: dynamic = 1;
        while ((i <= m))
        {
          write(0, cpp_char(" "));
          i += 1;
        }
      }
      write(cpp_char("\n"));
      continue;
    }
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        ans[i] = 0;
        i += 1;
      }
    }
    var cur: dynamic = 1;
    var vec: dynamic = cpp_construct((n + 1), 0);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = (need[(i - 1)] + 1);
          while ((j <= need[i]))
          {
            var id: dynamic = G.matchR[j];
            if (vec[i])
            {
              ans[id] = cpp_assign(ans[vec[i]], "=", cpp_update(cur, "++"));
              vec[i] = 0;
            } else
            {
              vec[i] = id;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        if ((!ans[i]))
        {
          ans[i] = cpp_update(cur, "++");
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        write(ans[i], cpp_char(" "));
        i += 1;
      }
    }
    write(cpp_char("\n"));
  }
  return 0;
}
