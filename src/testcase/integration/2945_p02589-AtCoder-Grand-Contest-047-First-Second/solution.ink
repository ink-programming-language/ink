// Translated from solution.cpp.

var N = cpp_expression("#include <iostream> #");

var ans = 0;

var ch = cpp_array(27, 1000005);

var cnt: dynamic;

var len: dynamic;

var tot = 1;

var ed = cpp_array(1000005);

var siz = cpp_array(1000005);

var fa = cpp_array(1000005);

var f = cpp_array(27, 1000005);

var id = cpp_array(1000005);

var s = cpp_array(1000005);

func add(s: dynamic)
{
  var len = strlen(s);
  var x = 1;
  {
    var i = (len - 1);
    while ((i >= 0))
    {
      var f = (s[i] - cpp_char("a"));
      if ((!ch[x][f]))
      {
        ch[x][f] = cpp_update(tot, "++");
        fa[tot] = x;
        id[tot] = f;
      }
      x = ch[x][f];
      i -= 1;
    }
  }
  ed[x] += 1;
}

func dfs(x: dynamic)
{
  if ((x == 0))
  {
    return;
  }
  siz[x] = ed[x];
  {
    var i = 0;
    while ((i < 26))
    {
      var v = ch[x][i];
      dfs(v);
      siz[x] += siz[v];
      f[x][i] += siz[v];
      {
        var j = 0;
        while ((j < 26))
        {
          if ((j != i))
          {
            f[x][j] += f[v][j];
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%s", s);
      add(s);
      i += 1;
    }
  }
  dfs(1);
  {
    var i = 2;
    while ((i <= tot))
    {
      if (ed[i])
      {
        ans += (f[fa[i]][id[i]] * ed[i]);
      }
      i += 1;
    }
  }
  write((ans - n), "\n");
  return 0;
}
