// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var cnt: dynamic;

var a = cpp_array(1000005);

var v = cpp_array(1000005);

var mp: dynamic;

func dfs(x: dynamic)
{
  {
    var i = 0;
    while ((i < v[x].size()))
    {
      dfs(v[x][i]);
      i += 1;
    }
  }
  cnt += 1;
  a[x] = cnt;
}

func main()
{
  read(n, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((i > k))
      {
        mp.erase(a[(i - k)]);
      }
      v[(mp.upper_bound(a[i]))->second].push_back(i);
      mp[a[i]] = i;
      i += 1;
    }
  }
  dfs(0);
  {
    var i = 1;
    while ((i <= n))
    {
      write(a[i], "\n");
      i += 1;
    }
  }
  return 0;
}
