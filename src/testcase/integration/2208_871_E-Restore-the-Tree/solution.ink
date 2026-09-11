// Translated from solution.cpp.

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

var INF: dynamic = 1e18;

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var vertex: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(200);

var st: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(30000);

var dist: dynamic = cpp_array(30000, 200);

func dfs(idx: dynamic, now: dynamic, from_cpp: dynamic) -> dynamic
{
  for (var to: dynamic in g[now])
  {
    if ((to == from_cpp))
    {
      continue;
    }
    dist[idx][to] = (dist[idx][now] + 1);
    dfs(idx, to, now);
  }
}

var appear: dynamic = cpp_uninitialized();

var children: dynamic = cpp_uninitialized();

class UnionFind
{
  var par: dynamic = cpp_uninitialized();
  var rank: dynamic = cpp_uninitialized();
  var Size: dynamic = cpp_uninitialized();
  func UnionFind(n: dynamic = 1) -> dynamic
  {
      init(n);
    }
  func init(n: dynamic = 1) -> dynamic
  {
      par.resize((n + 1));
      rank.resize((n + 1));
      Size.resize((n + 1));
      {
        var i: dynamic = 0;
        while ((i <= n))
        {
          par[i] = i;
          rank[i] = 0;
          Size[i] = 1;
          i += 1;
        }
      }
    }
  func root(x: dynamic) -> dynamic
  {
      if ((par[x] == x))
      {
        return x;
      } else
      {
        var r: dynamic = root(par[x]);
        return cpp_assign(par[x], "=", r);
      }
    }
  func issame(x: dynamic, y: dynamic) -> dynamic
  {
      return (root(x) == root(y));
    }
  func merge(x: dynamic, y: dynamic) -> dynamic
  {
      x = root(x);
      y = root(y);
      if ((x == y))
      {
        return false;
      }
      if ((rank[x] < rank[y]))
      {
        swap(x, y);
      }
      if ((rank[x] == rank[y]))
      {
        rank[x] += 1;
      }
      par[y] = x;
      Size[x] += Size[y];
      return true;
    }
  func size(x: dynamic) -> dynamic
  {
      return Size[root(x)];
    }
}

var match_cpp: dynamic = cpp_array(30000);

var TL: dynamic = 2980;

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var start: dynamic = chrono.steady_clock.now();
  var nowtimer: dynamic = cpp_uninitialized();
  read(N, K);
  vertex.resize(K, -1);
  {
    var k: dynamic = 0;
    while ((k < K))
    {
      d[k].resize(N);
      {
        var i: dynamic = 0;
        while ((i < N))
        {
          read(d[k][i]);
          if ((d[k][i] == 0))
          {
            if ((vertex[k] != -1))
            {
              write(-1, "\n");
              return 0;
            } else
            {
              vertex[k] = i;
            }
          }
          i += 1;
        }
      }
      if ((vertex[k] == -1))
      {
        write(-1, "\n");
        return 0;
      }
      appear.insert(vertex[k]);
      k += 1;
    }
  }
  {
    var k: dynamic = 1;
    while ((k < K))
    {
      var mini: dynamic = d[0][vertex[k]];
      var v: dynamic = cpp_construct((mini + 1), -1);
      {
        var i: dynamic = 0;
        while ((i < N))
        {
          if (((d[0][i] + d[k][i]) < mini))
          {
            write(-1, "\n");
          }
          if (((d[0][i] + d[k][i]) == mini))
          {
            if ((v[d[0][i]] != -1))
            {
              write(-1, "\n");
              return 0;
            }
            v[d[0][i]] = i;
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i <= mini))
        {
          if ((v[i] == -1))
          {
            write(-1, "\n");
            return 0;
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < mini))
        {
          var a: dynamic = v[i];
          var b: dynamic = v[(i + 1)];
          if ((a > b))
          {
            swap(a, b);
          }
          st.insert([a, b]);
          i += 1;
        }
      }
      k += 1;
    }
  }
  for (var tmp: dynamic in st)
  {
    if ((!uni.merge(tmp.first, tmp.second)))
    {
      write(-1, "\n");
      return 0;
    } else
    {
      g[tmp.first].push_back(tmp.second);
      g[tmp.second].push_back(tmp.first);
      appear.insert(tmp.first);
      appear.insert(tmp.second);
    }
  }
  var mp: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((appear.find(i) == appear.end()))
      {
        i += 1;
        continue;
      }
      var mini: dynamic = 1e9;
      {
        var j: dynamic = 0;
        while ((j < K))
        {
          v[j] = d[j][i];
          chmin(mini, v[j]);
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < K))
        {
          v[j] -= mini;
          j += 1;
        }
      }
      mp.push_back([v, i]);
      i += 1;
    }
  }
  mp.push_back([[999999999], -1]);
  sort(mp.begin(), mp.end());
  var sub: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((appear.find(i) != appear.end()))
      {
        i += 1;
        continue;
      }
      var mini: dynamic = 1e9;
      {
        var j: dynamic = 0;
        while ((j < K))
        {
          v[j] = d[j][i];
          chmin(mini, v[j]);
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < K))
        {
          v[j] -= mini;
          j += 1;
        }
      }
      sub.push_back([v, i]);
      i += 1;
    }
  }
  sort(sub.begin(), sub.end());
  var idx: dynamic = 0;
  if (((cpp_assign(nowtimer, "=", chrono.duration_cast((chrono.steady_clock.now() - start)).count())) > TL))
  {
    assert(0);
  }
  {
    var i: dynamic = 0;
    while ((i < sub.size()))
    {
      if (((cpp_assign(nowtimer, "=", chrono.duration_cast((chrono.steady_clock.now() - start)).count())) > TL))
      {
        return 0;
      }
      while ((mp[idx].first < sub[i].first))
      {
        idx += 1;
      }
      if ((mp[idx].first == sub[i].first))
      {
        match_cpp[sub[i].second] = mp[idx].second;
      } else
      {
        write(-1, "\n");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((appear.find(i) != appear.end()))
      {
        i += 1;
        continue;
      }
      var p: dynamic = match_cpp[i];
      var len: dynamic = (d[0][i] - d[0][p]);
      if ((len <= 0))
      {
        write(-1, "\n");
        return 0;
      }
      children[p].push_back([len, i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if (children[i].empty())
      {
        i += 1;
        continue;
      }
      var v: dynamic = children[i];
      sort(v.begin(), v.end());
      if ((v[0].first != 1))
      {
        write(-1, "\n");
        return 0;
      }
      var Last: dynamic = i;
      st.insert([i, v[0].second]);
      g[Last].push_back(v[0].second);
      g[v[0].second].push_back(Last);
      {
        var i: dynamic = 1;
        while ((i < v.size()))
        {
          if ((v[i].first > (v[(i - 1)].first + 1)))
          {
            write(-1, "\n");
            return 0;
          } else if ((v[i].first == v[(i - 1)].first))
          {
            st.insert([Last, v[i].second]);
            g[Last].push_back(v[i].second);
            g[v[i].second].push_back(Last);
          } else
          {
            Last = v[(i - 1)].second;
            st.insert([Last, v[i].second]);
            g[Last].push_back(v[i].second);
            g[v[i].second].push_back(Last);
          }
          i += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < K))
    {
      dfs(i, vertex[i], -1);
      {
        var j: dynamic = 0;
        while ((j < N))
        {
          if ((dist[i][j] != d[i][j]))
          {
            write(-1, "\n");
            return 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  for (var e: dynamic in st)
  {
    write((e.first + 1), " ", (e.second + 1), "\n");
  }
  return 0;
}
