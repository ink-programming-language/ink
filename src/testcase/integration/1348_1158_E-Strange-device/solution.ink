// Translated from solution.cpp.

var N = 1100;

class Edge
{
  var to: dynamic;
  var next: dynamic;
}

var edge = cpp_array((N * 2));

var head = cpp_array(N);

var num: dynamic;

func add_edge(a: dynamic, b: dynamic)
{
  edge[cpp_update(num, "++")] = [b, head[a]];
  head[a] = num;
}

var n: dynamic;

var q = cpp_array(N);

var a = cpp_array(N);

var a0 = cpp_array(N);

var ccc: dynamic;

var s = cpp_array(N);

func dfs(x: dynamic, d: dynamic, f: dynamic, ff: dynamic)
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
    var i = head[x];
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

func wen()
{
  printf("? ");
  {
    var i = 1;
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
    var i = 1;
    while ((i <= n))
    {
      a[i] = (s[i] - cpp_char("0"));
      i += 1;
    }
  }
}

var vis = cpp_array(N);

var vv = cpp_array(N);

var t = cpp_array(N);

var tt = cpp_array(N);

var nt = cpp_array(N);

var td = cpp_array(3);

var st = cpp_array(N);

var dep = cpp_array(N);

var ea = cpp_array(N);

var ce: dynamic;

var ee = cpp_array(N);

func main()
{
  scanf("%d", (&n));
  t[0].push_back(1);
  {
    var i = 1;
    while ((i <= n))
    {
      tt[0].push_back(i);
      i += 1;
    }
  }
  var cnt = (n - 1);
  vis[0] = cpp_assign(vis[1022], "=", cpp_assign(vv[1], "=", true));
  var la: dynamic;
  while (cnt)
  {
    if ((la == 20))
    {
      break;
    }
    la += 1;
    var top = 0;
    {
      var i = 0;
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
      var i = 1;
      while ((i <= (top - 1)))
      {
        if ((((i & 1)) && tt[st[i]].size()))
        {
          var tmp = (((st[(i + 1)] - st[i])) / 2);
          vis[(st[i] + tmp)] = true;
          {
            var j = 0;
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
      var i = 1;
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
      var i = 1;
      while ((i <= (top - 1)))
      {
        if ((((i & 1)) && tt[st[i]].size()))
        {
          var tmp = (((st[(i + 1)] - st[i])) / 2);
          {
            var j = 0;
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
      var i = 1;
      while ((i <= (top - 1)))
      {
        if (((!((i & 1))) && tt[st[i]].size()))
        {
          var tmp = (((st[(i + 1)] - st[i])) / 2);
          vis[(st[i] + tmp)] = true;
          {
            var j = 0;
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
      var i = 1;
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
      var i = 1;
      while ((i <= (top - 1)))
      {
        if (((!((i & 1))) && tt[st[i]].size()))
        {
          var tmp = (((st[(i + 1)] - st[i])) / 2);
          {
            var j = 0;
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
      var i = 0;
      while ((i <= n))
      {
        tt[i] = nt[i];
        nt[i].clear();
        i += 1;
      }
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      if (t[i].size())
      {
        {
          var j = 0;
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
    var i = 0;
    while ((i <= 2))
    {
      if (td[i].size())
      {
        {
          var j = 1;
          while ((j <= 10))
          {
            {
              var k = 0;
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
              var k = 1;
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
          var j = 1;
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
    var i = 1;
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
