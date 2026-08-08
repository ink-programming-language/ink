// Translated from solution.cpp.

var N = 4050;

var n: dynamic;

var G = cpp_array((N << 1));

var mark = cpp_array((N << 1));

var S = cpp_array((N << 1));

var c: dynamic;

func dfs(x: dynamic)
{
  if (mark[(x ^ 1)])
  {
    return 0;
  }
  if (mark[x])
  {
    return 1;
  }
  mark[x] = 1;
  S[cpp_update(c, "++")] = x;
  {
    var i = 0;
    while ((i < G[x].size()))
    {
      if ((!dfs(G[x][i])))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func init(xd: dynamic)
{
  n = xd;
  {
    var i = 0;
    while ((i < (2 * n)))
    {
      G[i].clear();
      i += 1;
    }
  }
  memset(mark, 0, cpp_sizeof((mark)));
}

func add_clause(x: dynamic, xval: dynamic, y: dynamic, yval: dynamic)
{
  x = (((x << 1)) + xval);
  y = (((y << 1)) + yval);
  G[(x ^ 1)].push_back(y);
  G[(y ^ 1)].push_back(x);
}

func add_xor(x: dynamic, y: dynamic)
{
  add_clause(x, 0, y, 0);
  add_clause(x, 1, y, 1);
}

func add_xnor(x: dynamic, y: dynamic)
{
  add_clause(x, 1, y, 0);
  add_clause(x, 0, y, 1);
}

func add_true(x: dynamic)
{
  add_clause(x, 1, x, 1);
}

func add_false(x: dynamic)
{
  add_clause(x, 0, x, 0);
}

func solve()
{
  {
    var i = 0;
    while ((i < (2 * n)))
    {
      if (((!mark[i]) && (!mark[(i + 1)])))
      {
        c = 0;
        if ((!dfs(i)))
        {
          while (c)
          {
            mark[S[cpp_update(c, "--")]] = 0;
          }
          if ((!dfs((i + 1))))
          {
            return 0;
          }
        }
      }
      i += 2;
    }
  }
  return 1;
}

func fail()
{
  write(-1, "\n");
  exit(0);
}

var mat = cpp_array(2005, 2005);

var tab = cpp_array(2005);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var first: dynamic;
  read(first);
  {
    var i = 0;
    while ((i < (first)))
    {
      {
        var j = 0;
        while ((j < (first)))
        {
          var x: dynamic;
          read(x);
          mat[i][j] = cpp_cast(((x == cpp_char("1"))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (first)))
    {
      {
        var j = 0;
        while ((j < (first)))
        {
          var a: dynamic;
          read(a);
          mat[i][j] ^= cpp_cast(((a == cpp_char("1"))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (first)))
    {
      var x: dynamic;
      read(x);
      tab[i] = cpp_cast(((x == cpp_char("1"))));
      i += 1;
    }
  }
  init((2 * first));
  {
    var i = 0;
    while ((i < (first)))
    {
      {
        var j = 0;
        while ((j < (first)))
        {
          var a = tab[j];
          var b = tab[i];
          if ((mat[i][j] == 1))
          {
            if (((a == 0) && (b == 0)))
            {
              fail();
            }
            if (((a == 1) && (b == 0)))
            {
              add_true(i);
            }
            if (((a == 0) && (b == 1)))
            {
              add_true((first + j));
            }
            if (((a == 1) && (b == 1)))
            {
              add_xor(i, (first + j));
            }
          } else
          {
            if (((a == 1) && (b == 0)))
            {
              add_false(i);
            }
            if (((a == 0) && (b == 1)))
            {
              add_false((first + j));
            }
            if (((a == 1) && (b == 1)))
            {
              add_xnor(i, (first + j));
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((solve() == false))
  {
    write(-1, "\n");
  } else
  {
    var ans: dynamic;
    {
      var i = 0;
      while ((i < (first)))
      {
        if (mark[(((i << 1)) + 1)])
        {
          ans.push_back(make_pair(0, i));
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < (first)))
      {
        if (mark[(((((i + first)) << 1)) + 1)])
        {
          ans.push_back(make_pair(1, i));
        }
        i += 1;
      }
    }
    write(cpp_cast((ans).size()), "\n");
    for (var i in (ans))
    {
      if ((i.first == 0))
      {
        write("row ", i.second, "\n");
      } else
      {
        write("col ", i.second, "\n");
      }
    }
  }
  return 0;
}
