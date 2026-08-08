// Translated from solution.cpp.

var mxn = 400005;

var maxnn = 100005;

var mod = (1e9 + 7);

var ans: dynamic;

var n: dynamic;

var m: dynamic;

var num: dynamic;

var cir: dynamic;

var r: dynamic;

var cnt: dynamic;

var head = cpp_array(maxnn);

var que = cpp_array(maxnn);

class edge
{
  var to: dynamic;
  var next: dynamic;
  var w: dynamic;
}

var f = cpp_array((mxn << 2));

var p = cpp_array(70);

var circle = cpp_array(mxn);

var dis = cpp_array(maxnn);

var dig = cpp_array(2);

var pw = cpp_array(105);

func add(u: dynamic, v: dynamic, w: dynamic)
{
  f[cpp_update(cnt, "++")].to = v;
  f[cnt].w = w;
  f[cnt].next = head[u];
  head[u] = cnt;
}

func dfs(u: dynamic, fa: dynamic, now: dynamic)
{
  dis[u] = now;
  que[cpp_update(num, "++")] = u;
  {
    var i = head[u];
    while (i)
    {
      var v = f[i].to;
      if ((v == fa))
      {
        i = f[i].next;
        continue;
      }
      if ((dis[v] == -1))
      {
        dfs(v, u, (dis[u] ^ f[i].w));
      } else
      {
        circle[cpp_update(cir, "++")] = ((dis[u] ^ dis[v]) ^ f[i].w);
      }
      i = f[i].next;
    }
  }
}

func init()
{
  var i: dynamic;
  var j: dynamic;
  r = 0;
  memset(p, 0, cpp_sizeof(p));
  {
    i = 1;
    while ((i <= cir))
    {
      var x = circle[i];
      {
        j = 62;
        while ((j >= 0))
        {
          if ((!((x >> j))))
          {
            j -= 1;
            continue;
          }
          if ((!p[j]))
          {
            p[j] = x;
            break;
          }
          x ^= p[j];
          j -= 1;
        }
      }
      i += 1;
    }
  }
  {
    j = 0;
    while ((j <= 62))
    {
      if (p[j])
      {
        r += 1;
      }
      j += 1;
    }
  }
}

func calc()
{
  init();
  var i: dynamic;
  var j: dynamic;
  {
    j = 0;
    while ((j <= 62))
    {
      var flag = 0;
      dig[0] = cpp_assign(dig[1], "=", 0);
      {
        i = 1;
        while ((i <= num))
        {
          dig[(((dis[que[i]] >> j)) & 1)] += 1;
          i += 1;
        }
      }
      {
        i = 0;
        while ((i <= 62))
        {
          if ((((p[i] >> j)) & 1))
          {
            flag = 1;
            break;
          }
          i += 1;
        }
      }
      var now = (((((dig[0] * ((dig[0] - 1))) / 2) + ((dig[1] * ((dig[1] - 1))) / 2))) % mod);
      if (flag)
      {
        if (r)
        {
          now = ((now * pw[(r - 1)]) % mod);
        }
        now = ((now * pw[j]) % mod);
        ans = (((ans + now)) % mod);
      }
      now = ((dig[0] * dig[1]) % mod);
      if (flag)
      {
        if (r)
        {
          now = ((now * pw[(r - 1)]) % mod);
        }
      } else
      {
        now = ((now * pw[r]) % mod);
      }
      now = ((now * pw[j]) % mod);
      ans = (((ans + now)) % mod);
      j += 1;
    }
  }
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var u: dynamic;
  var v: dynamic;
  var w: dynamic;
  memset(dis, -1, cpp_sizeof(dis));
  pw[0] = 1;
  {
    j = 1;
    while ((j <= 100))
    {
      pw[j] = ((pw[(j - 1)] * 2) % mod);
      j += 1;
    }
  }
  scanf("%d%d", (&n), (&m));
  {
    i = 1;
    while ((i <= m))
    {
      scanf("%d%d%lld", (&u), (&v), (&w));
      add(u, v, w);
      add(v, u, w);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if ((dis[i] == -1))
      {
        num = cpp_assign(cir, "=", 0);
        dfs(i, 0, 0);
        calc();
      }
      i += 1;
    }
  }
  printf("%lld", ans);
  return 0;
}
