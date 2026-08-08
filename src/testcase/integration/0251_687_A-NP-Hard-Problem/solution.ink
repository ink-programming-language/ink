// Translated from solution.cpp.

var p: dynamic;

var color: dynamic;

var count_0: dynamic;

var count_1: dynamic;

var n: dynamic;

var m: dynamic;

func dfs(x: dynamic)
{
  var now: dynamic;
  var q: dynamic;
  color[x] = 0;
  count_0 += 1;
  q.push_back(x);
  while ((q.size() > 0))
  {
    now = q.back();
    q.pop_back();
    {
      var i = 0;
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

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(n, m);
  p.resize(n);
  color.resize(n, -1);
  {
    var i = 0;
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
    var i = 0;
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
  var k = 0;
  write(count_0, "\n");
  {
    var j = 0;
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
    var j = 0;
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
