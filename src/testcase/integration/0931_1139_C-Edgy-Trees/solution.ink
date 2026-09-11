// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var v: dynamic = cpp_construct(100001);

var vis: dynamic = cpp_construct(100001);

func binpow(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while ((b > 0))
  {
    if ((b & 1))
    {
      res = (((res * a)) % mod);
    }
    a = (((a * a)) % mod);
    b >>= 1;
  }
  return (res % mod);
}

func dfs(src: dynamic) -> dynamic
{
  var st: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  st.push(src);
  vis[src] = 1;
  cnt += 1;
  while ((!st.empty()))
  {
    var x: dynamic = st.top();
    st.pop();
    if ((!vis[x]))
    {
      cnt += 1;
      vis[x] = 1;
    }
    {
      var i: dynamic = 0;
      while ((i < v[x].size()))
      {
        if ((vis[v[x][i]] == 0))
        {
          st.push(v[x][i]);
        }
        i += 1;
      }
    }
  }
  return cnt;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  fill(vis.begin(), vis.end(), 0);
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      v[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      read(x, y, c);
      if ((c == 0))
      {
        v[x].push_back(y);
        v[y].push_back(x);
      }
      i += 1;
    }
  }
  var ans: dynamic = binpow(n, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        ans = ((((mod + ans) - binpow(dfs(i), k))) % mod);
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
