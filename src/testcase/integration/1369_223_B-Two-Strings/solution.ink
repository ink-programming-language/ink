// Translated from solution.cpp.

var N: dynamic = 200005;

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var last: dynamic = cpp_array(26);

var G: dynamic = cpp_array(26);

var vis: dynamic = cpp_array(N);

var lc: dynamic = cpp_array(N);

var rc: dynamic = cpp_array(N);

func main() -> dynamic
{
  read(a, b);
  var m: dynamic = strlen(b);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      G[(b[i] - cpp_char("a"))].push_back(i);
      i += 1;
    }
  }
  memset(last, -1, cpp_sizeof((last)));
  var n: dynamic = strlen(a);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = (a[i] - cpp_char("a"));
      lc[i] = last[x];
      var p: dynamic = (last[x] + 1);
      if ((p == G[x].size()))
      {
        i += 1;
        continue;
      }
      var pos: dynamic = G[x][p];
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
    var i: dynamic = 0;
    while ((i < 26))
    {
      last[i] = G[i].size();
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      var x: dynamic = (a[i] - cpp_char("a"));
      rc[i] = last[x];
      var p: dynamic = (last[x] - 1);
      if ((p == -1))
      {
        i -= 1;
        continue;
      }
      var pos: dynamic = G[x][p];
      if ((((pos + 1) == m) || vis[(pos + 1)]))
      {
        rc[i] = cpp_update(last[x], "--");
        vis[pos] = 1;
      }
      i -= 1;
    }
  }
  var flag: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((lc[i] < rc[i]))
      {
        flag = 0;
      }
      i += 1;
    }
  }
  puts( (flag) ? "Yes" : "No");
}
