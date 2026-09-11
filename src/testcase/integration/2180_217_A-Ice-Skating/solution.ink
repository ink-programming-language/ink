// Translated from solution.cpp.

var xy: dynamic = cpp_uninitialized();

var used: dynamic = [false];

var n: dynamic = 0;

var ans: dynamic = -1;

func dfs(v: dynamic) -> dynamic
{
  used[v] = true;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (used[i])
      {
        i += 1;
        continue;
      }
      if (((xy[i].first == xy[v].first) || (xy[i].second == xy[v].second)))
      {
        dfs(i);
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  xy.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(xy[i].first, xy[i].second);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((!used[i]))
      {
        ans += 1;
        dfs(i);
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
