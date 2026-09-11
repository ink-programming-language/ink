// Translated from solution.cpp.

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var adj: dynamic = cpp_array(1000005);

var adj_r: dynamic = cpp_array(1000005);

var use: dynamic = cpp_array(1000005);

var use_r: dynamic = cpp_array(1000005);

var ans: dynamic = cpp_uninitialized();

func dfs(node: dynamic, s: dynamic) -> dynamic
{
  use[node] = 1;
  for (var it: dynamic in adj[node])
  {
    if ((use[it] == 0))
    {
      dfs(it, s);
    }
  }
  s.push(node);
  return;
}

func dfs_r(node: dynamic, sr: dynamic) -> dynamic
{
  use_r[node] = 1;
  for (var it: dynamic in adj_r[node])
  {
    if ((use_r[it] == 0))
    {
      dfs_r(it, sr);
    }
  }
  sr.push(node);
  return;
}

func init(n: dynamic) -> dynamic
{
  memset(use, 0, (cpp_sizeof(dynamic) * n));
  memset(use_r, 0, (cpp_sizeof(dynamic) * n));
  ans.clear();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      adj[i].clear();
      adj_r[i].clear();
      i += 1;
    }
  }
  return;
}

func main() -> dynamic
{
  scanf("%d", (&t));
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      scanf("%d%d", (&n), (&m));
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var jm: dynamic = cpp_uninitialized();
          var cm: dynamic = cpp_uninitialized();
          scanf("%d%d", (&jm), (&cm));
          if ((jm == cm))
          {
            j += 1;
            continue;
          }
          adj[jm].push_back(cm);
          adj_r[cm].push_back(jm);
          j += 1;
        }
      }
      var s: dynamic = cpp_uninitialized();
      dfs(1, s);
      if ((s.size() < n))
      {
        var ss: dynamic = s.size();
        {
          var j: dynamic = 0;
          while ((j < ss))
          {
            ans.push_back(s.top());
            s.pop();
            j += 1;
          }
        }
        printf("Yes\n");
        printf("%d %d\n", ans.size(), (n - ans.size()));
        sort(ans.begin(), ans.end());
        {
          var j: dynamic = 0;
          while ((j < ans.size()))
          {
            printf("%d ", ans[j]);
            j += 1;
          }
        }
        printf("\n");
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            if ((use[j] == 0))
            {
              printf("%d ", j);
            }
            j += 1;
          }
        }
        printf("\n");
        init((n + 1));
        i += 1;
        continue;
      }
      var count: dynamic = 0;
      var off: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          var idx: dynamic = s.top();
          s.pop();
          if (use_r[idx])
          {
            j += 1;
            continue;
          }
          var sr: dynamic = cpp_uninitialized();
          dfs_r(idx, sr);
          if ((sr.size() == n))
          {
            printf("No\n");
            init((n + 1));
            break;
          } else
          {
            count += sr.size();
            if ((count == n))
            {
              var ssr: dynamic = sr.size();
              {
                var k: dynamic = 0;
                while ((k < ssr))
                {
                  ans.push_back(sr.top());
                  off.insert(sr.top());
                  sr.pop();
                  k += 1;
                }
              }
              sort(ans.begin(), ans.end());
              printf("Yes\n");
              printf("%d %d\n", ans.size(), (n - ans.size()));
              {
                var j: dynamic = 0;
                while ((j < ans.size()))
                {
                  printf("%d ", ans[j]);
                  j += 1;
                }
              }
              printf("\n");
              {
                var j: dynamic = 1;
                while ((j <= n))
                {
                  if ((off.find(j) == off.end()))
                  {
                    printf("%d ", j);
                  }
                  j += 1;
                }
              }
              printf("\n");
              init((n + 1));
              break;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
