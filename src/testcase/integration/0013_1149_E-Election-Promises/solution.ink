// Translated from solution.cpp.

var N: dynamic = (2e5 + 100);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var h: dynamic = cpp_array(N);

var lev: dynamic = cpp_array(N);

var Xor: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(N);

var deg: dynamic = cpp_array(N);

var nxt: dynamic = cpp_array(N);

var rnxt: dynamic = cpp_array(N);

var vec: dynamic = cpp_array(N);

var s: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(h[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      rnxt[y].push_back(x);
      nxt[x].push_back(y);
      deg[x] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var u: dynamic = q.front();
    q.pop();
    for (var v: dynamic in nxt[u])
    {
      cnt[lev[v]] += 1;
    }
    while (cnt[lev[u]])
    {
      lev[u] += 1;
    }
    Xor[lev[u]] ^= h[u];
    for (var v: dynamic in nxt[u])
    {
      cnt[lev[v]] -= 1;
    }
    for (var v: dynamic in rnxt[u])
    {
      if ((!cpp_update(deg[v], "--")))
      {
        q.push(v);
      }
    }
  }
  {
    var i: dynamic = n;
    while ((~i))
    {
      if (Xor[i])
      {
        puts("WIN");
        {
          var u: dynamic = 1;
          while ((u <= n))
          {
            if (((lev[u] == i) && (h[u] > ((h[u] ^ Xor[i])))))
            {
              h[u] ^= Xor[i];
              for (var v: dynamic in nxt[u])
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
          var j: dynamic = 1;
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
