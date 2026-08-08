// Translated from solution.cpp.

var mod = 1000000007;

var v = cpp_construct(100001);

var vis = cpp_construct(100001);

func binpow(a: dynamic, b: dynamic)
{
  var res = 1;
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

func dfs(src: dynamic)
{
  var st: dynamic;
  var cnt = 0;
  st.push(src);
  vis[src] = 1;
  cnt += 1;
  while ((!st.empty()))
  {
    var x = st.top();
    st.pop();
    if ((!vis[x]))
    {
      cnt += 1;
      vis[x] = 1;
    }
    {
      var i = 0;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  fill(vis.begin(), vis.end(), 0);
  {
    var i = 0;
    while ((i < v.size()))
    {
      v[i].clear();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var x: dynamic;
      var y: dynamic;
      var c: dynamic;
      read(x, y, c);
      if ((c == 0))
      {
        v[x].push_back(y);
        v[y].push_back(x);
      }
      i += 1;
    }
  }
  var ans = binpow(n, k);
  {
    var i = 1;
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
