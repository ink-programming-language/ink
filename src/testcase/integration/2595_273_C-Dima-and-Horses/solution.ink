// Translated from solution.cpp.

var maxn: dynamic = 300005;

var adj: dynamic = cpp_array(maxn);

var deg: dynamic = cpp_array(maxn);

var c: dynamic = cpp_array(maxn);

func dfs(u: dynamic, p: dynamic, col: dynamic) -> dynamic
{
  c[u] = col;
  for (var v: dynamic in adj[u])
  {
    if (((v == p) || (c[v] != -1)))
    {
      continue;
    }
    dfs(v, p, (!col));
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      c[i] = -1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((c[i] == -1))
      {
        dfs(i, 0, 0);
      }
      i += 1;
    }
  }
  {
    var veces: dynamic = 0;
    while ((veces < 1))
    {
      var change: dynamic = false;
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          var mismo: dynamic = 0;
          var u: dynamic = i;
          for (var v: dynamic in adj[u])
          {
            if ((c[v] == c[u]))
            {
              mismo += 1;
            }
          }
          if ((mismo > 1))
          {
            change = true;
            var q: dynamic = cpp_uninitialized();
            q.push(u);
            while ((!q.empty()))
            {
              var curr: dynamic = q.front();
              q.pop();
              var igual: dynamic = 0;
              for (var v: dynamic in adj[curr])
              {
                if ((c[v] == c[curr]))
                {
                  igual += 1;
                }
              }
              if ((igual < 2))
              {
                continue;
              }
              c[curr] = (!c[curr]);
              for (var v: dynamic in adj[curr])
              {
                var mismo2: dynamic = 0;
                for (var w: dynamic in adj[v])
                {
                  if ((c[w] == c[v]))
                  {
                    mismo2 += 1;
                  }
                }
                if ((mismo2 > 1))
                {
                  q.push(v);
                }
              }
            }
          }
          i += 1;
        }
      }
      if ((!change))
      {
        break;
      }
      veces += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      assert((c[i] >= 0));
      write(c[i]);
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
