// Translated from solution.cpp.

class yts
{
  var x: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var ne: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(4000010);

class yts2
{
  var x: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var ne: dynamic = cpp_uninitialized();
}

var E: dynamic = cpp_array(4000010);

class PP
{
  var x: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var vec1: dynamic = cpp_array(1000010);

var vec2: dynamic = cpp_array(1000010);

var v: dynamic = cpp_array(2000010);

var V: dynamic = cpp_array(2000010);

var scc: dynamic = cpp_array(2000010);

var dfn: dynamic = cpp_array(2000010);

var low: dynamic = cpp_array(2000010);

var st: dynamic = cpp_array(2000010);

var q: dynamic = cpp_array(2000010);

var du: dynamic = cpp_array(2000010);

var f: dynamic = cpp_array(2000010);

var ID: dynamic = cpp_array(2000010);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var rnum: dynamic = cpp_uninitialized();

var num: dynamic = cpp_uninitialized();

var dfs_cnt: dynamic = cpp_uninitialized();

var scc_cnt: dynamic = cpp_uninitialized();

var top: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.x < b.x);
}

func go(x: dynamic, y: dynamic) -> dynamic
{
  return ((((x - 1)) * m) + y);
}

func reput(x: dynamic, y: dynamic) -> dynamic
{
  rnum += 1;
  E[rnum].x = x;
  E[rnum].t = y;
  du[y] += 1;
  E[rnum].ne = V[x];
  V[x] = rnum;
}

func put(x: dynamic, y: dynamic, l: dynamic) -> dynamic
{
  num += 1;
  e[num].x = x;
  e[num].t = y;
  e[num].l = l;
  e[num].ne = v[x];
  v[x] = num;
}

func tarjan(x: dynamic) -> dynamic
{
  dfn[x] = cpp_assign(low[x], "=", cpp_update(dfs_cnt, "++"));
  st[cpp_update(top, "++")] = x;
  {
    var i: dynamic = v[x];
    while (i)
    {
      var y: dynamic = e[i].t;
      if ((!dfn[y]))
      {
        tarjan(y);
        low[x] = min(low[x], low[y]);
      } else if ((!scc[y]))
      {
        low[x] = min(low[x], dfn[y]);
      }
      i = e[i].ne;
    }
  }
  if ((dfn[x] == low[x]))
  {
    var y: dynamic = cpp_uninitialized();
    scc_cnt += 1;
    while (true)
    {
      y = st[cpp_update(top, "--")];
      scc[y] = scc_cnt;
      if (!(((y != x))))
      {
        break;
      }
    }
  }
}

func rebuild() -> dynamic
{
  {
    var x: dynamic = 1;
    while ((x <= cnt))
    {
      {
        var i: dynamic = v[x];
        while (i)
        {
          var y: dynamic = e[i].t;
          if ((scc[x] != scc[y]))
          {
            reput(scc[x], scc[y]);
          }
          i = e[i].ne;
        }
      }
      x += 1;
    }
  }
}

func tp() -> dynamic
{
  var h: dynamic = 0;
  var w: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= scc_cnt))
    {
      if ((du[i] == 0))
      {
        q[cpp_update(w, "++")] = i;
      }
      i += 1;
    }
  }
  while ((h != w))
  {
    var x: dynamic = q[cpp_update(h, "++")];
    f[x] += 1;
    {
      var i: dynamic = V[x];
      while (i)
      {
        var y: dynamic = E[i].t;
        f[y] = max(f[y], f[x]);
        du[y] -= 1;
        if ((du[y] == 0))
        {
          q[cpp_update(w, "++")] = y;
        }
        i = E[i].ne;
      }
    }
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          var x: dynamic = cpp_uninitialized();
          scanf("%d", (&x));
          vec1[i].push_back([x, j]);
          vec2[j].push_back([x, i]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  cnt = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      sort(vec1[i].begin(), vec1[i].end(), cmp);
      var now: dynamic = -1;
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((vec1[i][j].x != now))
          {
            now = vec1[i][j].x;
            cnt += 1;
            if (j)
            {
              put((cnt - 1), cnt, 1);
            }
          }
          ID[go(i, vec1[i][j].id)] = cnt;
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
      sort(vec2[i].begin(), vec2[i].end(), cmp);
      var now: dynamic = -1;
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((vec2[i][j].x != now))
          {
            now = vec2[i][j].x;
            cnt += 1;
            if (j)
            {
              put((cnt - 1), cnt, 1);
            }
          }
          var y: dynamic = ID[go(vec2[i][j].id, i)];
          put(cnt, y, 0);
          put(y, cnt, 0);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= cnt))
    {
      if ((!dfn[i]))
      {
        tarjan(i);
      }
      i += 1;
    }
  }
  rebuild();
  tp();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          printf("%d ", f[scc[ID[go(i, j)]]]);
          j += 1;
        }
      }
      printf("\n");
      i += 1;
    }
  }
  return 0;
}
