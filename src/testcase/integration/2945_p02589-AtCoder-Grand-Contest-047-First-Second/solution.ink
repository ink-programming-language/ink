// Translated from solution.cpp.

var N: dynamic = cpp_expression("#include <iostream> #");

var ans: dynamic = 0;

var ch: dynamic = cpp_array(27, 1000005);

var cnt: dynamic = cpp_uninitialized();

var len: dynamic = cpp_uninitialized();

var tot: dynamic = 1;

var ed: dynamic = cpp_array(1000005);

var siz: dynamic = cpp_array(1000005);

var fa: dynamic = cpp_array(1000005);

var f: dynamic = cpp_array(27, 1000005);

var id: dynamic = cpp_array(1000005);

var s: dynamic = cpp_array(1000005);

func add(s: dynamic) -> dynamic
{
  var len: dynamic = strlen(s);
  var x: dynamic = 1;
  {
    var i: dynamic = (len - 1);
    while ((i >= 0))
    {
      var f: dynamic = (s[i] - cpp_char("a"));
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

func dfs(x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    return;
  }
  siz[x] = ed[x];
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      var v: dynamic = ch[x][i];
      dfs(v);
      siz[x] += siz[v];
      f[x][i] += siz[v];
      {
        var j: dynamic = 0;
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%s", s);
      add(s);
      i += 1;
    }
  }
  dfs(1);
  {
    var i: dynamic = 2;
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
