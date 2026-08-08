// Translated from solution.cpp.

var str = cpp_array(100010);

var line = cpp_array(27, 27);

var p = cpp_array(27);

var vis = cpp_array(27);

var in_cpp = cpp_array(27);

var out = cpp_array(27);

var ans: dynamic;

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", str);
      var len = strlen(str);
      {
        var i = 0;
        while ((i < (len - 1)))
        {
          if ((str[i] == str[(i + 1)]))
          {
            printf("NO\n");
            return 0;
          }
          line[(str[i] - cpp_char("a"))][(str[(i + 1)] - cpp_char("a"))] = true;
          i += 1;
        }
      }
      if ((len == 1))
      {
        p[(str[0] - cpp_char("a"))] = true;
      }
      i += 1;
    }
  }
  ans = "";
  {
    var i = 0;
    while ((i < 26))
    {
      {
        var j = 0;
        while ((j < 26))
        {
          if (line[i][j])
          {
            out[i] += 1;
            in_cpp[j] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 26))
    {
      if (((in_cpp[i] > 1) || (out[i] > 1)))
      {
        printf("NO\n");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 26))
    {
      if (((out[i] != 0) && (in_cpp[i] == 0)))
      {
        if ((!dfs(i)))
        {
          printf("NO\n");
          return 0;
        }
      }
      if (((p[i] && (in_cpp[i] == 0)) && (out[i] == 0)))
      {
        ans += ((i + cpp_char("a")));
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 26))
    {
      if ((((in_cpp[i] != 0) && (out[i] != 0)) && (!vis[i])))
      {
        printf("NO\n");
        return 0;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

func dfs(value: dynamic)
{
  vis[value] = true;
  ans += ((value + cpp_char("a")));
  {
    var i = 0;
    while ((i < 26))
    {
      if (line[value][i])
      {
        return dfs(i);
      }
      i += 1;
    }
  }
  return true;
}
