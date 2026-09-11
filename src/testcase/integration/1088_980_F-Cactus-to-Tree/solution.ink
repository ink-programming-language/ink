// Translated from solution.cpp.

var inf: dynamic = cpp_cast(1e9);

var linf: dynamic = cpp_cast(1e18);

var mod: dynamic = (cpp_cast(1e9) + 7);

var eps: dynamic = cpp_cast(1e-8);

var maxn: dynamic = (cpp_cast(5e5) + 5);

var pi: dynamic = acos(-1);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var cnt_cyc: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(maxn);

var cyc: dynamic = cpp_array(maxn);

var tup: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_array(maxn);

var ans1: dynamic = cpp_array(maxn);

var cyc_head: dynamic = cpp_array(maxn);

var p: dynamic = cpp_array(maxn);

var d: dynamic = cpp_array(maxn);

var dc: dynamic = cpp_array(maxn);

var dc_up: dynamic = cpp_array(maxn);

var a: dynamic = cpp_array(maxn);

var cycv: dynamic = cpp_array(maxn);

func dfs0(v: dynamic, pr: dynamic = -1) -> dynamic
{
  p[v] = pr;
  t[v] = 1;
  tup[v] = 1;
  for (var to: dynamic in a[v])
  {
    if ((to == pr))
    {
      continue;
    }
    if ((t[to] && (tup[to] == 0)))
    {
      continue;
    }
    if (t[to])
    {
      cnt_cyc += 1;
      cyc[to] = cnt_cyc;
      var x: dynamic = v;
      cycv[cnt_cyc].push_back(to);
      while ((cyc[x] == 0))
      {
        cyc[x] = cnt_cyc;
        cycv[cnt_cyc].push_back(x);
        x = p[x];
      }
      cyc_head[cnt_cyc] = to;
    } else
    {
      dfs0(to, v);
    }
  }
  if ((cyc[v] == 0))
  {
    cnt_cyc += 1;
    cyc[v] = cnt_cyc;
    cyc_head[cnt_cyc] = v;
    cycv[cnt_cyc].push_back(v);
  }
  tup[v] = 0;
}

func dfs_cyc0(cv: dynamic, cpr: dynamic = -1) -> dynamic
{
  for (var v: dynamic in cycv[cv])
  {
    for (var to: dynamic in a[v])
    {
      if (((cyc[to] == cpr) || (cyc[to] == cv)))
      {
        continue;
      }
      dfs_cyc0(cyc[to], cv);
      d[v] = max(d[v], (dc[cyc[to]] + 1));
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((cycv[cv]).size()))))
    {
      dc[cv] = max(dc[cv], (d[cycv[cv][i]] + min(i, ((cpp_cast((cycv[cv]).size())) - i))));
      i += 1;
    }
  }
}

func dfs_cyc1(cv: dynamic, cpr: dynamic = -1) -> dynamic
{
  if (((cpp_cast((cycv[cv]).size())) >= 3))
  {
    var b: dynamic = cpp_uninitialized();
    {
      var j: dynamic = 0;
      while ((j < 3))
      {
        {
          var i: dynamic = 0;
          while ((i < (cpp_cast((cycv[cv]).size()))))
          {
            b.push_back(d[cycv[cv][i]]);
            i += 1;
          }
        }
        j += 1;
      }
    }
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    l = ((cpp_cast((cycv[cv]).size())) - ((cpp_cast((cycv[cv]).size())) / 2));
    r = (((cpp_cast((cycv[cv]).size())) + ((cpp_cast((cycv[cv]).size())) / 2)) - ((((cpp_cast((cycv[cv]).size())) % 2) == 0)));
    var ql: dynamic = cpp_uninitialized();
    var qr: dynamic = cpp_uninitialized();
    var pl: dynamic = 0;
    var pr: dynamic = 0;
    {
      var i: dynamic = l;
      while ((i <= ((cpp_cast((cycv[cv]).size())) - 1)))
      {
        ql.insert(((b[i] + (cpp_cast((cycv[cv]).size()))) - i));
        i += 1;
      }
    }
    {
      var i: dynamic = ((cpp_cast((cycv[cv]).size())) + 1);
      while ((i <= r))
      {
        qr.insert(((b[i] + i) - (cpp_cast((cycv[cv]).size()))));
        i += 1;
      }
    }
    {
      var i: dynamic = (cpp_cast((cycv[cv]).size()));
      while ((i <= ((2 * (cpp_cast((cycv[cv]).size()))) - 1)))
      {
        var v: dynamic = cycv[cv][(i - (cpp_cast((cycv[cv]).size())))];
        ans1[v] = max(((*ql.begin()) + pl), ((*qr.begin()) + pr));
        ql.erase(ql.find((((i - l) + b[l]) - pl)));
        pl += 1;
        l += 1;
        ql.insert(((1 + b[i]) - pl));
        qr.erase(qr.find(((1 + b[(i + 1)]) - pr)));
        pr -= 1;
        r += 1;
        qr.insert((((r - ((i + 1))) + b[r]) - pr));
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < (cpp_cast((cycv[cv]).size()))))
      {
        var v: dynamic = cycv[cv][i];
        ans1[v] = max(ans1[v], (min(i, ((cpp_cast((cycv[cv]).size())) - i)) + dc_up[cv]));
        ans[v] = max(d[v], ans1[v]);
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 0;
      while ((i < (cpp_cast((cycv[cv]).size()))))
      {
        var v: dynamic = cycv[cv][i];
        ans1[v] = (min(i, ((cpp_cast((cycv[cv]).size())) - i)) + dc_up[cv]);
        {
          var j: dynamic = 0;
          while ((j < (cpp_cast((cycv[cv]).size()))))
          {
            if ((i == j))
            {
              j += 1;
              continue;
            }
            var u: dynamic = cycv[cv][j];
            var dist: dynamic = min(abs((i - j)), ((min(i, j) + (cpp_cast((cycv[cv]).size()))) - max(i, j)));
            ans1[v] = max(ans1[v], (d[u] + dist));
            j += 1;
          }
        }
        ans[v] = max(d[v], ans1[v]);
        i += 1;
      }
    }
  }
  for (var v: dynamic in cycv[cv])
  {
    var qm: dynamic = cpp_uninitialized();
    for (var to: dynamic in a[v])
    {
      if (((cyc[to] == cpr) || (cyc[to] == cv)))
      {
        continue;
      }
      qm.insert((-((dc[cyc[to]] + 1))));
      if (((cpp_cast((qm).size())) > 2))
      {
        qm.erase(cpp_update((cpp_update(qm.begin(), "++")), "++"));
      }
    }
    for (var to: dynamic in a[v])
    {
      if (((cyc[to] == cpr) || (cyc[to] == cv)))
      {
        continue;
      }
      if (((cpp_cast((qm).size())) == 1))
      {
        dc_up[cyc[to]] = (ans1[v] + 1);
      } else
      {
        var it: dynamic = qm.begin();
        if (((dc[cyc[to]] + 1) == (-(*it))))
        {
          it += 1;
        }
        dc_up[cyc[to]] = (max(ans1[v], (-(*it))) + 1);
      }
      dfs_cyc1(cyc[to], cv);
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      x -= 1;
      y -= 1;
      a[x].push_back(y);
      a[y].push_back(x);
      i += 1;
    }
  }
  memset((cyc), 0, cpp_sizeof((cyc)));
  cnt_cyc = 0;
  memset((t), 0, cpp_sizeof((t)));
  dfs0(0);
  memset((ans), 0, cpp_sizeof((ans)));
  memset((ans1), 0, cpp_sizeof((ans1)));
  memset((d), 0, cpp_sizeof((d)));
  memset((dc), 0, cpp_sizeof((dc)));
  memset((dc_up), 0, cpp_sizeof((dc_up)));
  dfs_cyc0(cyc[0]);
  dfs_cyc1(cyc[0]);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
