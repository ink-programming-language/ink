// Translated from solution.cpp.

var inf = 1e9;

class Matching
{
  var n: dynamic;
  var matchL: dynamic;
  var matchR: dynamic;
  var dist: dynamic;
  var seen: dynamic;
  var ke: dynamic;
  func Matching(n: dynamic)
  {
      this->n = cpp_construct(n);
      this->matchL = cpp_construct((n + 1));
      this->matchR = cpp_construct((n + 1));
      this->dist = cpp_construct((n + 1));
      this->seen = cpp_construct((n + 1), false);
      this->ke = cpp_construct((n + 1));
    }
  func addEdge(u: dynamic, v: dynamic)
  {
      ke[u].push_back(v);
    }
  func bfs()
  {
      var qu: dynamic;
      {
        var u = 1;
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
        var u = qu.front();
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
  func dfs(u: dynamic)
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
  func match_cpp()
  {
      var res = 0;
      while (bfs())
      {
        {
          var u = 1;
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

var n: dynamic;

var m: dynamic;

var k: dynamic;

var from_cpp = cpp_array(605);

var to = cpp_array(605);

var need = cpp_array(605);

var sneed: dynamic;

var ans = cpp_array(605);

func main(argument_0: dynamic)
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var ntest: dynamic;
  read(ntest);
  while (cpp_update(ntest, "--"))
  {
    read(n, m, k);
    {
      var i = 1;
      while ((i <= n))
      {
        need[i] = 0;
        i += 1;
      }
    }
    {
      var i = 1;
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
      var i = 1;
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
        var i = 1;
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
      var i = 1;
      while ((i <= n))
      {
        need[i] += need[(i - 1)];
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= m))
      {
        var u = from_cpp[i];
        var v = to[i];
        {
          var j = (need[(u - 1)] + 1);
          while ((j <= need[u]))
          {
            G.addEdge(i, j);
            j += 1;
          }
        }
        {
          var j = (need[(v - 1)] + 1);
          while ((j <= need[v]))
          {
            G.addEdge(i, j);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var cnt = G.match_cpp();
    if ((cnt < sneed))
    {
      {
        var i = 1;
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
      var i = 1;
      while ((i <= m))
      {
        ans[i] = 0;
        i += 1;
      }
    }
    var cur = 1;
    var vec = cpp_construct((n + 1), 0);
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = (need[(i - 1)] + 1);
          while ((j <= need[i]))
          {
            var id = G.matchR[j];
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
      var i = 1;
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
      var i = 1;
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
