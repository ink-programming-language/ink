// Translated from solution.cpp.

var N = (2e5 + 100);

var n: dynamic;

var m: dynamic;

var h = cpp_array(N);

var lev = cpp_array(N);

var Xor = cpp_array(N);

var cnt = cpp_array(N);

var deg = cpp_array(N);

var nxt = cpp_array(N);

var rnxt = cpp_array(N);

var vec = cpp_array(N);

var s: dynamic;

var q: dynamic;

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(h[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      rnxt[y].push_back(x);
      nxt[x].push_back(y);
      deg[x] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!deg[i]))
      {
        q.push(i);
      }
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var u = q.front();
    q.pop();
    for (var v in nxt[u])
    {
      cnt[lev[v]] += 1;
    }
    while (cnt[lev[u]])
    {
      lev[u] += 1;
    }
    Xor[lev[u]] ^= h[u];
    for (var v in nxt[u])
    {
      cnt[lev[v]] -= 1;
    }
    for (var v in rnxt[u])
    {
      if ((!cpp_update(deg[v], "--")))
      {
        q.push(v);
      }
    }
  }
  {
    var i = n;
    while ((~i))
    {
      if (Xor[i])
      {
        puts("WIN");
        {
          var u = 1;
          while ((u <= n))
          {
            if (((lev[u] == i) && (h[u] > ((h[u] ^ Xor[i])))))
            {
              h[u] ^= Xor[i];
              for (var v in nxt[u])
              {
                if (Xor[lev[v]])
                {
                  h[v] ^= Xor[lev[v]];
                  Xor[lev[v]] = 0;
                }
              }
              break;
            }
            u += 1;
          }
        }
        {
          var j = 1;
          while ((j <= n))
          {
            write(h[j], " ");
            j += 1;
          }
        }
        puts("");
        return 0;
      }
      i -= 1;
    }
  }
  puts("LOSE");
  return 0;
}
