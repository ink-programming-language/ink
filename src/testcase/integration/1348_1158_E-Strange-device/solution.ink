// Translated from solution.cpp.

var N: dynamic = 1100;

class Edge
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var edge: dynamic = cpp_array((N * 2));

var head: dynamic = cpp_array(N);

var num: dynamic = cpp_uninitialized();

func add_edge(a: dynamic, b: dynamic) -> dynamic
{
  edge[cpp_update(num, "++")] = [b, head[a]];
  head[a] = num;
}

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var a0: dynamic = cpp_array(N);

var ccc: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(N);

func dfs(x: dynamic, d: dynamic, f: dynamic, ff: dynamic) -> dynamic
{
  if ((x != ff))
  {
    a[x] = 1;
  }
  if ((!d))
  {
    return;
  }
  {
    var i: dynamic = head[x];
    while (i)
    {
      if ((edge[i].to != f))
      {
        dfs(edge[i].to, (d - 1), x, ff);
      }
      i = edge[i].next;
    }
  }
}

func wen() -> dynamic
{
  printf("? ");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d ", min(q[i], (n - 1)));
      i += 1;
    }
  }
  printf("\n");
  fflush(stdout);
  scanf("%s", (s + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = (s[i] - cpp_char("0"));
      i += 1;
    }
  }
}

var vis: dynamic = cpp_array(N);

var vv: dynamic = cpp_array(N);

var t: dynamic = cpp_array(N);

var tt: dynamic = cpp_array(N);

var nt: dynamic = cpp_array(N);

var td: dynamic = cpp_array(3);

var st: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

var ea: dynamic = cpp_array(N);

var ce: dynamic = cpp_uninitialized();

var ee: dynamic = cpp_array(N);

func main() -> dynamic
{
  scanf("%d", (&n));
  t[0].push_back(1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      tt[0].push_back(i);
      i += 1;
    }
  }
  var cnt: dynamic = (n - 1);
  vis[0] = cpp_assign(vis[1022], "=", cpp_assign(vv[1], "=", true));
  var la: dynamic = cpp_uninitialized();
  while (cnt)
  {
    if ((la == 20))
    {
      break;
    }
    la += 1;
    var top: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i <= 1022))
      {
        if (vis[i])
        {
          st[cpp_update(top, "++")] = i;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= (top - 1)))
      {
        if ((((i & 1)) && tt[st[i]].size()))
        {
          var tmp: dynamic = (((st[(i + 1)] - st[i])) / 2);
          vis[(st[i] + tmp)] = true;
          {
            var j: dynamic = 0;
            while ((j <= (t[st[i]].size() - 1)))
            {
              q[t[st[i]][j]] = tmp;
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
    wen();
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        a0[i] = a[i];
        if (q[i])
        {
          q[i] -= 1;
        }
        i += 1;
      }
    }
    wen();
    memset(q, 0, cpp_sizeof((q)));
    {
      var i: dynamic = 1;
      while ((i <= (top - 1)))
      {
        if ((((i & 1)) && tt[st[i]].size()))
        {
          var tmp: dynamic = (((st[(i + 1)] - st[i])) / 2);
          {
            var j: dynamic = 0;
            while ((j <= (tt[st[i]].size() - 1)))
            {
              if ((vv[tt[st[i]][j]] || a[tt[st[i]][j]]))
              {
                nt[st[i]].push_back(tt[st[i]][j]);
              } else
              {
                nt[(st[i] + tmp)].push_back(tt[st[i]][j]);
              }
              if ((((!vv[tt[st[i]][j]]) && a0[tt[st[i]][j]]) && (!a[tt[st[i]][j]])))
              {
                cnt -= 1;
                vv[tt[st[i]][j]] = true;
                t[(st[i] + tmp)].push_back(tt[st[i]][j]);
              }
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= (top - 1)))
      {
        if (((!((i & 1))) && tt[st[i]].size()))
        {
          var tmp: dynamic = (((st[(i + 1)] - st[i])) / 2);
          vis[(st[i] + tmp)] = true;
          {
            var j: dynamic = 0;
            while ((j <= (t[st[i]].size() - 1)))
            {
              q[t[st[i]][j]] = tmp;
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
    wen();
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        a0[i] = a[i];
        if (q[i])
        {
          q[i] -= 1;
        }
        i += 1;
      }
    }
    wen();
    memset(q, 0, cpp_sizeof((q)));
    {
      var i: dynamic = 1;
      while ((i <= (top - 1)))
      {
        if (((!((i & 1))) && tt[st[i]].size()))
        {
          var tmp: dynamic = (((st[(i + 1)] - st[i])) / 2);
          {
            var j: dynamic = 0;
            while ((j <= (tt[st[i]].size() - 1)))
            {
              if ((vv[tt[st[i]][j]] || a[tt[st[i]][j]]))
              {
                nt[st[i]].push_back(tt[st[i]][j]);
              } else
              {
                nt[(st[i] + tmp)].push_back(tt[st[i]][j]);
              }
              if ((((!vv[tt[st[i]][j]]) && a0[tt[st[i]][j]]) && (!a[tt[st[i]][j]])))
              {
                cnt -= 1;
                vv[tt[st[i]][j]] = true;
                t[(st[i] + tmp)].push_back(tt[st[i]][j]);
              }
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i <= n))
      {
        tt[i] = nt[i];
        nt[i].clear();
        i += 1;
      }
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      if (t[i].size())
      {
        {
          var j: dynamic = 0;
          while ((j <= (t[i].size() - 1)))
          {
            dep[t[i][j]] = i;
            td[(i % 3)].push_back(t[i][j]);
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= 2))
    {
      if (td[i].size())
      {
        {
          var j: dynamic = 1;
          while ((j <= 10))
          {
            {
              var k: dynamic = 0;
              while ((k <= (td[i].size() - 1)))
              {
                if ((td[i][k] & ((1 << ((j - 1))))))
                {
                  q[td[i][k]] = 1;
                }
                k += 1;
              }
            }
            wen();
            memset(q, 0, cpp_sizeof((q)));
            {
              var k: dynamic = 1;
              while ((k <= n))
              {
                if ((a[k] && ((dep[k] % 3) == (((i + 1)) % 3))))
                {
                  ea[k] |= ((1 << ((j - 1))));
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            if ((((dep[j] % 3) == (((i + 1)) % 3)) && ea[j]))
            {
              ee[cpp_update(ce, "++")] = make_pair(ea[j], j);
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  printf("!\n");
  {
    var i: dynamic = 1;
    while ((i <= (n - 1)))
    {
      printf("%d %d", ee[i].first, ee[i].second);
      if ((i != (n - 1)))
      {
        printf("\n");
      }
      i += 1;
    }
  }
  return 0;
}
