// Translated from solution.cpp.

var mxn: dynamic = 400005;

var maxnn: dynamic = 100005;

var mod: dynamic = (1e9 + 7);

var ans: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var num: dynamic = cpp_uninitialized();

var cir: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var head: dynamic = cpp_array(maxnn);

var que: dynamic = cpp_array(maxnn);

class edge
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
}

var f: dynamic = cpp_array((mxn << 2));

var p: dynamic = cpp_array(70);

var circle: dynamic = cpp_array(mxn);

var dis: dynamic = cpp_array(maxnn);

var dig: dynamic = cpp_array(2);

var pw: dynamic = cpp_array(105);

func add(u: dynamic, v: dynamic, w: dynamic) -> dynamic
{
  f[cpp_update(cnt, "++")].to = v;
  f[cnt].w = w;
  f[cnt].next = head[u];
  head[u] = cnt;
}

func dfs(u: dynamic, fa: dynamic, now: dynamic) -> dynamic
{
  dis[u] = now;
  que[cpp_update(num, "++")] = u;
  {
    var i: dynamic = head[u];
    while (i)
    {
      var v: dynamic = f[i].to;
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

func init() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  r = 0;
  memset(p, 0, cpp_sizeof(p));
  {
    i = 1;
    while ((i <= cir))
    {
      var x: dynamic = circle[i];
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

func calc() -> dynamic
{
  init();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    j = 0;
    while ((j <= 62))
    {
      var flag: dynamic = 0;
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
      var now: dynamic = (((((dig[0] * ((dig[0] - 1))) / 2) + ((dig[1] * ((dig[1] - 1))) / 2))) % mod);
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

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
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
