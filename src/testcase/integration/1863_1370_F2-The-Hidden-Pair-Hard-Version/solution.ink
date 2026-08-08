// Translated from solution.cpp.

var INF = 2e9;

var ML = 4e18;

func query(a: dynamic)
{
  write("? ", a.size());
  for (var i in a)
  {
    write(" ", i);
  }
  write("\n");
  fflush(stdout);
}

func solve()
{
  var n: dynamic;
  read(n);
  var edges = cpp_construct((n + 1));
  {
    var i = (0);
    while ((i < ((n - 1))))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      edges[a].push_back(b);
      edges[b].push_back(a);
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      a[i] = (i + 1);
      i += 1;
    }
  }
  var x: dynamic;
  var d: dynamic;
  query(a);
  read(x, d);
  var q: dynamic;
  var rs: dynamic;
  q.push([x, 0]);
  var f = cpp_construct((n + 1));
  f[x] = 0;
  while ((!q.empty()))
  {
    var sz = q.size();
    var r: dynamic;
    {
      var i = (0);
      while ((i < (sz)))
      {
        var p = q.front();
        q.pop();
        r.push_back(p.first);
        for (var j in edges[p.first])
        {
          if ((j == p.second))
          {
            continue;
          }
          q.push([j, p.first]);
          f[j] = p.first;
        }
        i += 1;
      }
    }
    rs.push_back(r);
  }
  var low = (((d + 1)) / 2);
  var high = min(d, int_cpp((rs.size() - 1)));
  var ans = -1;
  var rx: dynamic;
  var rd: dynamic;
  while ((low < high))
  {
    var mid = ((((low + high) + 1)) / 2);
    query(rs[mid]);
    read(rx, rd);
    if ((rd == d))
    {
      low = mid;
      ans = rx;
    } else
    {
      high = (mid - 1);
    }
  }
  if ((ans == -1))
  {
    query(rs[high]);
    read(ans, rd);
  }
  var ok: dynamic;
  if ((low == d))
  {
    write("! ", x, " ", ans, "\n");
    read(ok);
    fflush(stdout);
    return;
  }
  var p = (d - low);
  var st: dynamic;
  var cur = ans;
  while (cur)
  {
    st.insert(cur);
    cur = f[cur];
  }
  var last: dynamic;
  for (var i in rs[p])
  {
    if ((!st.count(i)))
    {
      last.push_back(i);
    }
  }
  var res = -1;
  query(last);
  read(res, d);
  write("! ", res, " ", ans, "\n");
  read(ok);
  fflush(stdout);
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var T: dynamic;
  read(T);
  {
    var kase = 1;
    while ((kase <= T))
    {
      solve();
      kase += 1;
    }
  }
  return 0;
}
