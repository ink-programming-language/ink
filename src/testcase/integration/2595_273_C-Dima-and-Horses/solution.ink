// Translated from solution.cpp.

var maxn = 300005;

var adj = cpp_array(maxn);

var deg = cpp_array(maxn);

var c = cpp_array(maxn);

func dfs(u: dynamic, p: dynamic, col: dynamic)
{
  c[u] = col;
  for (var v in adj[u])
  {
    if (((v == p) || (c[v] != -1)))
    {
      continue;
    }
    dfs(v, p, (!col));
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      c[i] = -1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      adj[u].push_back(v);
      adj[v].push_back(u);
      i += 1;
    }
  }
  {
    var i = 1;
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
    var veces = 0;
    while ((veces < 1))
    {
      var change = false;
      {
        var i = 1;
        while ((i <= n))
        {
          var mismo = 0;
          var u = i;
          for (var v in adj[u])
          {
            if ((c[v] == c[u]))
            {
              mismo += 1;
            }
          }
          if ((mismo > 1))
          {
            change = true;
            var q: dynamic;
            q.push(u);
            while ((!q.empty()))
            {
              var curr = q.front();
              q.pop();
              var igual = 0;
              for (var v in adj[curr])
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
              for (var v in adj[curr])
              {
                var mismo2 = 0;
                for (var w in adj[v])
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
    var i = 1;
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
