// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000005);

var v: dynamic = cpp_array(1000005);

var mp: dynamic = cpp_uninitialized();

func dfs(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v[x].size()))
    {
      dfs(v[x][i]);
      i += 1;
    }
  }
  cnt += 1;
  a[x] = cnt;
}

func main() -> dynamic
{
  read(n, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(a[i], "\n");
      i += 1;
    }
  }
  return 0;
}
