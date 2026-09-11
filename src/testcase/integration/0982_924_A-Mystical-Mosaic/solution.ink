// Translated from solution.cpp.

var g: dynamic = cpp_array(500);

var rg: dynamic = cpp_array(500);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(500);

var curc: dynamic = 0;

var color: dynamic = cpp_array(500);

var color2: dynamic = cpp_array(500);

var c1: dynamic = cpp_array(500);

var c2: dynamic = cpp_array(500);

func no() -> dynamic
{
  write("No\n");
  exit(0);
}

func yes() -> dynamic
{
  write("Yes\n");
  exit(0);
}

func dfs(i: dynamic) -> dynamic
{
  c1[curc].insert(i);
  color[i] = curc;
  for (var j: dynamic in g[i])
  {
    if ((color2[j] && (color2[j] != curc)))
    {
      no();
    }
    if ((color2[j] == 0))
    {
      color2[j] = curc;
      dfs2(j);
    }
  }
}

func dfs2(j: dynamic) -> dynamic
{
  c2[curc].insert(j);
  color2[j] = curc;
  for (var i: dynamic in rg[j])
  {
    if ((color[i] && (color[i] != curc)))
    {
      no();
    }
    if ((color[i] == 0))
    {
      color[i] = curc;
      dfs(i);
    }
  }
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((s[i][j] == cpp_char("#")))
          {
            g[i].push_back(j);
            rg[j].push_back(i);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      curc += 1;
      if ((!color[i]))
      {
        dfs(i);
      }
      i += 1;
    }
  }
  {
    var j: dynamic = 0;
    while ((j < m))
    {
      curc += 1;
      if ((!color2[j]))
      {
        dfs2(j);
      }
      j += 1;
    }
  }
  {
    var c: dynamic = 1;
    while ((c <= curc))
    {
      var ii: dynamic = c1[c];
      var jj: dynamic = c2[c];
      for (var i: dynamic in ii)
      {
        for (var j: dynamic in jj)
        {
          if ((s[i][j] != cpp_char("#")))
          {
            no();
          }
        }
      }
      c += 1;
    }
  }
  write("Yes\n");
  return 0;
}
