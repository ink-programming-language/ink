// Translated from solution.cpp.

var INF: dynamic = 2e9;

var ML: dynamic = 4e18;

func query(a: dynamic) -> dynamic
{
  write("? ", a.size());
  for (var i: dynamic in a)
  {
    write(" ", i);
  }
  write("\n");
  fflush(stdout);
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var edges: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = (0);
    while ((i < ((n - 1))))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      edges[a].push_back(b);
      edges[b].push_back(a);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      a[i] = (i + 1);
      i += 1;
    }
  }
  var x: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  query(a);
  read(x, d);
  var q: dynamic = cpp_uninitialized();
  var rs: dynamic = cpp_uninitialized();
  q.push([x, 0]);
  var f: dynamic = cpp_construct((n + 1));
  f[x] = 0;
  while ((!q.empty()))
  {
    var sz: dynamic = q.size();
    var r: dynamic = cpp_uninitialized();
    {
      var i: dynamic = (0);
      while ((i < (sz)))
      {
        var p: dynamic = q.front();
        q.pop();
        r.push_back(p.first);
        for (var j: dynamic in edges[p.first])
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
  var low: dynamic = (((d + 1)) / 2);
  var high: dynamic = min(d, int_cpp((rs.size() - 1)));
  var ans: dynamic = -1;
  var rx: dynamic = cpp_uninitialized();
  var rd: dynamic = cpp_uninitialized();
  while ((low < high))
  {
    var mid: dynamic = ((((low + high) + 1)) / 2);
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
  var ok: dynamic = cpp_uninitialized();
  if ((low == d))
  {
    write("! ", x, " ", ans, "\n");
    read(ok);
    fflush(stdout);
    return;
  }
  var p: dynamic = (d - low);
  var st: dynamic = cpp_uninitialized();
  var cur: dynamic = ans;
  while (cur)
  {
    st.insert(cur);
    cur = f[cur];
  }
  var last: dynamic = cpp_uninitialized();
  for (var i: dynamic in rs[p])
  {
    if ((!st.count(i)))
    {
      last.push_back(i);
    }
  }
  var res: dynamic = -1;
  query(last);
  read(res, d);
  write("! ", res, " ", ans, "\n");
  read(ok);
  fflush(stdout);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var T: dynamic = cpp_uninitialized();
  read(T);
  {
    var kase: dynamic = 1;
    while ((kase <= T))
    {
      solve();
      kase += 1;
    }
  }
  return 0;
}
