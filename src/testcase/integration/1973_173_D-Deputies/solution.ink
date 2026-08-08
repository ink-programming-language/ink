// Translated from solution.cpp.

var fa = cpp_array(100005);

func makeset(n: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      fa[i] = i;
      i += 1;
    }
  }
}

func findset(u: dynamic)
{
  if ((fa[u] == u))
  {
    return fa[u];
  }
  return cpp_assign(fa[u], "=", findset(fa[u]));
}

func unionset(a: dynamic, b: dynamic)
{
  var u = findset(a);
  var v = findset(b);
  fa[u] = v;
}

var n: dynamic;

var m: dynamic;

var tot: dynamic;

var x = 1;

var y = 0;

var tot_white: dynamic;

var tot_black: dynamic;

var col = cpp_array(100005);

var id = cpp_array(100005);

var d = cpp_array(100005);

var used = cpp_array(100005);

var w: dynamic;

var b: dynamic;

var g = cpp_array(100005);

var v = cpp_array(100005);

var t = cpp_array(100005);

func pr()
{
  write("YES\n");
  {
    var i = 1;
    while ((i <= n))
    {
      write(id[i], cpp_char(" "));
      i += 1;
    }
  }
  exit(0);
}

func makenext(x: dynamic, y: dynamic)
{
  y += 1;
  x += (y / 3);
  y %= 3;
}

func dfs(u: dynamic)
{
  if ((col[u] == 1))
  {
    tot_white += 1;
  } else
  {
    tot_black += 1;
  }
  {
    var i = 0;
    while ((i < g[u].size()))
    {
      var v = g[u][i];
      if ((col[v] != (col[u] * -1)))
      {
        col[v] = (col[u] * -1);
        dfs(v);
      }
      i += 1;
    }
  }
}

func prepare()
{
  {
    var i = 1;
    while ((i <= n))
    {
      if ((col[i] == 0))
      {
        col[i] = 1;
        dfs(i);
      }
      i += 1;
    }
  }
  if (((tot_white % 3) == 2))
  {
    swap(tot_white, tot_black);
    {
      var i = 1;
      while ((i <= n))
      {
        col[i] *= -1;
        i += 1;
      }
    }
  }
}

func filp()
{
  {
    var i = 1;
    while ((i <= n))
    {
      if ((v[i].size() >= 1))
      {
        {
          var j = 0;
          while ((j < v[i].size()))
          {
            col[v[i][j]] *= -1;
            j += 1;
          }
        }
        break;
      }
      i += 1;
    }
  }
  tot_white = cpp_assign(tot_black, "=", 0);
  {
    var i = 1;
    while ((i <= n))
    {
      if ((col[i] == 1))
      {
        tot_white += 1;
      } else
      {
        tot_black += 1;
      }
      i += 1;
    }
  }
  if (((tot_white % 3) == 2))
  {
    swap(tot_white, tot_black);
    {
      var i = 1;
      while ((i <= n))
      {
        col[i] *= -1;
        i += 1;
      }
    }
  }
}

func make12(x: dynamic, y: dynamic)
{
  var ind: dynamic;
  {
    var i = 0;
    while ((i < w.size()))
    {
      if ((id[w[i]] == 0))
      {
        ind = w[i];
        break;
      }
      i += 1;
    }
  }
  id[ind] = x;
  makenext(x, y);
  {
    var i = 0;
    while ((i < b.size()))
    {
      if (((id[b[i]] == 0) && (t[ind][b[i]] == 0)))
      {
        id[b[i]] = x;
        makenext(x, y);
      }
      if ((x != id[ind]))
      {
        break;
      }
      i += 1;
    }
  }
}

func makeall(x: dynamic, y: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      if (((col[i] == 1) && (id[i] == 0)))
      {
        id[i] = x;
        makenext(x, y);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (((col[i] == -1) && (id[i] == 0)))
      {
        id[i] = x;
        makenext(x, y);
      }
      i += 1;
    }
  }
}

func check()
{
  w.clear();
  b.clear();
  {
    var i = 1;
    while ((i <= n))
    {
      if ((col[i] == 1))
      {
        w.push_back(i);
      } else
      {
        b.push_back(i);
      }
      i += 1;
    }
  }
  var f = 0;
  {
    var i = 0;
    while ((i < w.size()))
    {
      if ((d[w[i]] <= (tot_black - 2)))
      {
        swap(w[0], w[i]);
        f = 1;
        break;
      }
      i += 1;
    }
  }
  if (f)
  {
    make12(x, y);
    makeall(x, y);
    pr();
  }
  if ((tot_white >= 4))
  {
    var tot = 0;
    {
      var i = 0;
      while ((i < b.size()))
      {
        if (((d[b[i]] <= (tot_white - 2)) && (tot <= 1)))
        {
          swap(b[tot], b[i]);
          tot += 1;
        }
        i += 1;
      }
    }
    if ((tot == 2))
    {
      swap(tot_black, tot_white);
      swap(w, b);
      {
        var i = 1;
        while ((i <= n))
        {
          col[i] *= -1;
          i += 1;
        }
      }
      make12(x, y);
      make12(x, y);
      makeall(x, y);
      pr();
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  read(n, m);
  makeset(n);
  {
    var i = 1;
    var u: dynamic;
    var v: dynamic;
    while ((i <= m))
    {
      read(u, v);
      g[u].push_back(v);
      g[v].push_back(u);
      d[u] += 1;
      d[v] += 1;
      t[u][v] = cpp_assign(t[v][u], "=", 1);
      unionset(u, v);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((used[findset(i)] == 0))
      {
        used[findset(i)] = 1;
        tot += 1;
      }
      v[findset(i)].push_back(i);
      i += 1;
    }
  }
  prepare();
  if ((((tot_white % 3) == 0) && ((tot_black % 3) == 0)))
  {
    makeall(x, y);
    pr();
  }
  if ((tot == 2))
  {
    check();
  } else
  {
    check();
  }
  write("NO");
  return 0;
}
