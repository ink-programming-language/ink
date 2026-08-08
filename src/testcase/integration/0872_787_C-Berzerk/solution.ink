// Translated from solution.cpp.

var inf = 0x3f3f3f3f;

var eps = 1e-8;

var n: dynamic;

var k1: dynamic;

var k2: dynamic;

var cnt = cpp_array(14005);

var edges = cpp_array(2);

var res = cpp_array(14005);

var a = cpp_array(7005);

var b = cpp_array(7005);

var rev = cpp_array(2);

var visited = cpp_array(14005);

var q: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  while ((cin >> n))
  {
    read(k1);
    {
      var i = 0;
      while ((i < k1))
      {
        read(a[i]);
        rev[0].push_back(a[i]);
        i += 1;
      }
    }
    read(k2);
    {
      var i = 0;
      while ((i < k2))
      {
        read(b[i]);
        rev[1].push_back(b[i]);
        i += 1;
      }
    }
    edges[0] = k2;
    edges[1] = k1;
    memset(cnt, 0, cpp_sizeof((cnt)));
    memset(visited, 0, cpp_sizeof((visited)));
    memset(res, -1, cpp_sizeof((res)));
    q.push(0);
    q.push(1);
    visited[0] = cpp_assign(visited[1], "=", 1);
    res[0] = cpp_assign(res[1], "=", 0);
    var cur: dynamic;
    var nxt: dynamic;
    while (q.size())
    {
      cur = q.front();
      q.pop();
      if ((res[cur] != -1))
      {
        {
          var i = 0;
          while ((i < rev[(cur % 2)].size()))
          {
            nxt = ((((((cur - ((rev[(cur % 2)][i] << 1))) + ((n << 1)))) % ((n << 1)))) ^ 1);
            if ((!visited[nxt]))
            {
              if (res[cur])
              {
                cnt[nxt] += 1;
              }
              if (((!res[cur]) || (cnt[nxt] == edges[(nxt % 2)])))
              {
                res[nxt] = (1 - res[cur]);
                visited[nxt] = 1;
                q.push(nxt);
              }
            }
            i += 1;
          }
        }
      }
    }
    {
      var i = 1;
      while ((i < n))
      {
        if ((res[((i << 1) | 1)] == -1))
        {
          write("Loop ");
        } else if (res[((i << 1) | 1)])
        {
          write("Win ");
        } else
        {
          write("Lose ");
        }
        i += 1;
      }
    }
    write("\n");
    {
      var i = 1;
      while ((i < n))
      {
        if ((res[(i << 1)] == -1))
        {
          write("Loop ");
        } else if (res[(i << 1)])
        {
          write("Win ");
        } else
        {
          write("Lose ");
        }
        i += 1;
      }
    }
    write("\n");
    rev[0].clear();
    rev[1].clear();
  }
  return 0;
}
