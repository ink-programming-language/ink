// Translated from solution.cpp.

var xy: dynamic;

var used = [false];

var n = 0;

var ans = -1;

func dfs(v: dynamic)
{
  used[v] = true;
  {
    var i = 0;
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

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  xy.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(xy[i].first, xy[i].second);
      i += 1;
    }
  }
  {
    var i = 0;
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
