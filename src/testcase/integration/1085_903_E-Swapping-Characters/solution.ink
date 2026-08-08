// Translated from solution.cpp.

var N = 5005;

var mod = (1e17 + 7);

var k: dynamic;

var n: dynamic;

var s = cpp_array(N);

var high: dynamic;

var idx: dynamic;

var cnt = cpp_array(26);

func satisfy(x: dynamic)
{
  var ret = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((s[x][i] != s[idx][i]))
      {
        ret += 1;
      }
      i += 1;
    }
  }
  if (cpp_binary(cpp_binary((ret == 2), "or", (cpp_binary((ret == 0), "and", (high >= 2)))), "or", ((x == idx))))
  {
    return true;
  }
  return false;
}

func main()
{
  var occ: dynamic;
  ios.sync_with_stdio(false);
  read(k, n);
  {
    var i = 1;
    while ((i < (k + 1)))
    {
      read(s[i]);
      i += 1;
    }
  }
  var flag = true;
  {
    var i = 1;
    while ((i < k))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if ((s[i][j] != s[(i + 1)][j]))
          {
            idx = i;
            flag = false;
            occ = j;
            break;
          }
          j += 1;
        }
      }
      if ((!flag))
      {
        break;
      }
      i += 1;
    }
  }
  if (flag)
  {
    swap(s[1][0], s[1][1]);
    write(s[1], "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < n))
    {
      cnt[(s[1][i] - cpp_char("a"))] += 1;
      high = max(high, cnt[(s[1][i] - cpp_char("a"))]);
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i < (k + 1)))
    {
      var tmp = cpp_construct(26, 0);
      {
        var j = 0;
        while ((j < n))
        {
          tmp[(s[i][j] - cpp_char("a"))] += 1;
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < 26))
        {
          if ((tmp[j] != cnt[j]))
          {
            write("-1\n");
            return 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var yo = false;
  {
    var i = 0;
    while ((i < n))
    {
      if ((i == occ))
      {
        i += 1;
        continue;
      }
      swap(s[idx][i], s[idx][occ]);
      var ret = true;
      {
        var j = 1;
        while ((j < (k + 1)))
        {
          if ((!satisfy(j)))
          {
            ret = false;
            break;
          }
          j += 1;
        }
      }
      if (ret)
      {
        yo = true;
        break;
      }
      swap(s[idx][i], s[idx][occ]);
      i += 1;
    }
  }
  if (yo)
  {
    write(s[idx], "\n");
  } else
  {
    idx += 1;
    yo = false;
    {
      var i = 0;
      while ((i < n))
      {
        if ((i == occ))
        {
          i += 1;
          continue;
        }
        swap(s[idx][i], s[idx][occ]);
        var ret = true;
        {
          var j = 1;
          while ((j < (k + 1)))
          {
            if ((!satisfy(j)))
            {
              ret = false;
              break;
            }
            j += 1;
          }
        }
        if (ret)
        {
          yo = true;
          break;
        }
        swap(s[idx][i], s[idx][occ]);
        i += 1;
      }
    }
    if (yo)
    {
      write(s[idx], "\n");
      return 0;
    }
    write("-1\n");
  }
  return 0;
}
