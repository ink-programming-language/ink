// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var v: dynamic = cpp_array(N);

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var color: dynamic = cpp_array(N);

var visited: dynamic = cpp_array(N);

var ans: dynamic = 0;

var zero: dynamic = cpp_uninitialized();

var one: dynamic = cpp_uninitialized();

var ok: dynamic = 1;

func dfs(node: dynamic) -> dynamic
{
  for (var next: dynamic in v[node])
  {
    if (visited[next])
    {
      if ((color[next] == color[node]))
      {
        ok = 0;
        return;
      }
    } else
    {
      visited[next] = 1;
      color[next] = (color[node] ^ 1);
      one += color[next];
      zero += color[node];
      dfs(next);
    }
  }
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      read(a, b);
      v[a].emplace_back(b);
      v[b].emplace_back(a);
      i += 1;
    }
  }
  if ((m == 0))
  {
    write("3 ", ((((1 * n) * ((n - 1))) * ((n - 2))) / 6));
  } else
  {
    var mx: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        mx = max(mx, int_cpp(v[i].size()));
        i += 1;
      }
    }
    if ((mx == 1))
    {
      write("2 ", ((1 * m) * ((n - 2))));
    } else
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          if ((!visited[i]))
          {
            visited[i] = 1;
            color[i] = 1;
            one = 1;
            zero = 0;
            dfs(i);
            ans += (((1 * one) * ((one - 1))) / 2);
            ans += (((1 * zero) * ((zero - 1))) / 2);
          }
          i += 1;
        }
      }
      if (ok)
      {
        write("1 ", ans);
      } else
      {
        write("0 1");
      }
    }
  }
}
