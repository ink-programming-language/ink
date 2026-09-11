// Translated from solution.cpp.

var p: dynamic = cpp_uninitialized();

var color: dynamic = cpp_uninitialized();

var count_0: dynamic = cpp_uninitialized();

var count_1: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func dfs(x: dynamic) -> dynamic
{
  var now: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  color[x] = 0;
  count_0 += 1;
  q.push_back(x);
  while ((q.size() > 0))
  {
    now = q.back();
    q.pop_back();
    {
      var i: dynamic = 0;
      while ((i < p[now].size()))
      {
        if ((color[p[now][i]] > -1))
        {
          if ((color[p[now][i]] != (1 - color[now])))
          {
            return 1;
          }
        } else
        {
          color[p[now][i]] = (1 - color[now]);
          if (color[p[now][i]])
          {
            count_1 += 1;
          } else
          {
            count_0 += 1;
          }
          q.push_back(p[now][i]);
        }
        i += 1;
      }
    }
  }
  return 0;
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(n, m);
  p.resize(n);
  color.resize(n, -1);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(a, b);
      a -= 1;
      b -= 1;
      p[a].push_back(b);
      p[b].push_back(a);
      i += 1;
    }
  }
  count_0 = 0;
  count_1 = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (((color[i] == -1) && (p[i].size() > 0)))
      {
        if (dfs(i))
        {
          write(-1, "\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  var k: dynamic = 0;
  write(count_0, "\n");
  {
    var j: dynamic = 0;
    while ((j < n))
    {
      if ((color[j] == 0))
      {
        write((j + 1));
        k += 1;
        if ((k < count_0))
        {
          write(" ");
        } else
        {
          break;
        }
      }
      j += 1;
    }
  }
  write("\n");
  k = 0;
  write(count_1, "\n");
  {
    var j: dynamic = 0;
    while ((j < n))
    {
      if ((color[j] == 1))
      {
        write((j + 1));
        k += 1;
        if ((k < count_1))
        {
          write(" ");
        } else
        {
          break;
        }
      }
      j += 1;
    }
  }
  write("\n");
  return 0;
}
