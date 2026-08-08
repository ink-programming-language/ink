// Translated from solution.cpp.

var g = cpp_array(500);

var rg = cpp_array(500);

var n: dynamic;

var m: dynamic;

var s = cpp_array(500);

var curc = 0;

var color = cpp_array(500);

var color2 = cpp_array(500);

var c1 = cpp_array(500);

var c2 = cpp_array(500);

func no()
{
  write("No\n");
  exit(0);
}

func yes()
{
  write("Yes\n");
  exit(0);
}

func dfs(i: dynamic)
{
  c1[curc].insert(i);
  color[i] = curc;
  for (var j in g[i])
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

func dfs2(j: dynamic)
{
  c2[curc].insert(j);
  color2[j] = curc;
  for (var i in rg[j])
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

func main()
{
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(s[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
    var i = 0;
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
    var j = 0;
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
    var c = 1;
    while ((c <= curc))
    {
      var ii = c1[c];
      var jj = c2[c];
      for (var i in ii)
      {
        for (var j in jj)
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
