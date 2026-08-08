// Translated from solution.cpp.

var N = 200005;

var a = cpp_array(N);

var b = cpp_array(N);

var last = cpp_array(26);

var G = cpp_array(26);

var vis = cpp_array(N);

var lc = cpp_array(N);

var rc = cpp_array(N);

func main()
{
  read(a, b);
  var m = strlen(b);
  {
    var i = 0;
    while ((i < m))
    {
      G[(b[i] - cpp_char("a"))].push_back(i);
      i += 1;
    }
  }
  memset(last, -1, cpp_sizeof((last)));
  var n = strlen(a);
  {
    var i = 0;
    while ((i < n))
    {
      var x = (a[i] - cpp_char("a"));
      lc[i] = last[x];
      var p = (last[x] + 1);
      if ((p == G[x].size()))
      {
        i += 1;
        continue;
      }
      var pos = G[x][p];
      if (((0 == pos) || vis[(pos - 1)]))
      {
        lc[i] = cpp_update(last[x], "++");
        vis[pos] = 1;
      }
      i += 1;
    }
  }
  memset(vis, 0, cpp_sizeof((vis)));
  {
    var i = 0;
    while ((i < 26))
    {
      last[i] = G[i].size();
      i += 1;
    }
  }
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      var x = (a[i] - cpp_char("a"));
      rc[i] = last[x];
      var p = (last[x] - 1);
      if ((p == -1))
      {
        i -= 1;
        continue;
      }
      var pos = G[x][p];
      if ((((pos + 1) == m) || vis[(pos + 1)]))
      {
        rc[i] = cpp_update(last[x], "--");
        vis[pos] = 1;
      }
      i -= 1;
    }
  }
  var flag = 1;
  {
    var i = 0;
    while ((i < n))
    {
      if ((lc[i] < rc[i]))
      {
        flag = 0;
      }
      i += 1;
    }
  }
  puts(if (flag) "Yes" else "No");
}
