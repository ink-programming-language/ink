// Translated from solution.cpp.

var M: dynamic = 1000000007;

var LM: dynamic = (1 << 60);

func solve(edge: dynamic, k: dynamic) -> dynamic
{
  var n: dynamic = edge.size();
  var q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      deg[i] = edge[i].size();
      if ((deg[i] < k))
      {
        del[i] = true;
        q.push(i);
      }
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var p: dynamic = q.front();
    q.pop();
    for (var i: dynamic in edge[p])
    {
      if (del[i])
      {
        continue;
      }
      deg[i] -= 1;
      if ((deg[i] < k))
      {
        del[i] = true;
        q.push(i);
      }
    }
  }
  var res: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((!del[i]))
      {
        res.push_back(i);
      }
      i += 1;
    }
  }
  return res;
}

func solve2(edge: dynamic, k: dynamic) -> dynamic
{
  var n: dynamic = edge.size();
  var q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      deg[i] = edge[i].size();
      if ((deg[i] < (k - 1)))
      {
        del[i] = true;
        q.push(i);
      }
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var p: dynamic = q.front();
    q.pop();
    for (var i: dynamic in edge[p])
    {
      if (del[i])
      {
        continue;
      }
      deg[i] -= 1;
      if ((deg[i] < (k - 1)))
      {
        del[i] = true;
        q.push(i);
      }
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (((!del[i]) && (deg[i] == (k - 1))))
      {
        pushed[i] = true;
        q.push(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      for (var e: dynamic in edge[i])
      {
        edgeset[i].insert(e);
      }
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var p: dynamic = q.front();
    q.pop();
    if ((deg[p] == (k - 1)))
    {
      var v: dynamic = cpp_uninitialized();
      v.push_back(p);
      for (var i: dynamic in edge[p])
      {
        if ((!del[i]))
        {
          v.push_back(i);
        }
      }
      var ok: dynamic = true;
      {
        var i: dynamic = 0;
        while (((i < k) && ok))
        {
          {
            var j: dynamic = (i + 1);
            while ((j < k))
            {
              if ((!edgeset[v[i]].count(v[j])))
              {
                ok = false;
                break;
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      if (ok)
      {
        return v;
      }
    }
    for (var i: dynamic in edge[p])
    {
      if (del[i])
      {
        continue;
      }
      deg[i] -= 1;
      if (((deg[i] == (k - 1)) && (!pushed[i])))
      {
        pushed[i] = true;
        q.push(i);
      }
    }
    del[p] = true;
  }
  return [];
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  var T: dynamic = cpp_uninitialized();
  read(T);
  {
    var cpp_name: dynamic = 0;
    while ((cpp_name < T))
    {
      var n: dynamic = cpp_uninitialized();
      var m: dynamic = cpp_uninitialized();
      var k: dynamic = cpp_uninitialized();
      read(n, m, k);
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          var u: dynamic = cpp_uninitialized();
          var v: dynamic = cpp_uninitialized();
          read(u, v);
          u -= 1;
          v -= 1;
          edge[u].push_back(v);
          edge[v].push_back(u);
          i += 1;
        }
      }
      {
        var s0: dynamic = solve(edge, k);
        if ((s0.size() > 0))
        {
          write(1, cpp_char(" "), s0.size(), cpp_char("\n"));
          {
            var i: dynamic = 0;
            while ((i < cpp_cast(s0.size())))
            {
              write((s0[i] + 1), ( (((i + 1) < cpp_cast(s0.size()))) ? cpp_char(" ") : cpp_char("\n")));
              i += 1;
            }
          }
          cpp_name += 1;
          continue;
        }
      }
      {
        var s1: dynamic = solve2(edge, k);
        if ((s1.size() > 0))
        {
          write(2, cpp_char("\n"));
          {
            var i: dynamic = 0;
            while ((i < cpp_cast(s1.size())))
            {
              write((s1[i] + 1), ( (((i + 1) < cpp_cast(s1.size()))) ? cpp_char(" ") : cpp_char("\n")));
              i += 1;
            }
          }
          cpp_name += 1;
          continue;
        }
      }
      write(-1, cpp_char("\n"));
      cpp_name += 1;
    }
  }
  return 0;
}
